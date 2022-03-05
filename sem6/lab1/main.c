#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <sys/file.h>


#define LOCKFILE "/var/run/daemon.pid"
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

sigset_t mask;


int lockfile(int fd)
{
    struct flock fl;

    fl.l_type = F_WRLCK; 
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    
    return (fcntl(fd, F_SETLK, &fl));
}


int already_running(void)
{

    syslog(LOG_ERR, "Проверка на запуск только одной копии демона!");

    int fd;
    char buf[16];

    fd = open(LOCKFILE, O_RDWR | O_CREAT, LOCKMODE);

    if (fd < 0) {
        syslog(LOG_ERR, "Невозможно открыть %s: %s!", LOCKFILE, strerror(errno));
        exit(1);
    }

    syslog(LOG_WARNING, "Lock-файл открыт!");


    flock(fd, LOCK_EX | LOCK_UN);

    if (errno == EWOULDBLOCK) {
        syslog(LOG_ERR, "Невозможно установить блокировку на %s: %s!", LOCKFILE, strerror(errno));
        close(fd);
        exit(1);
    }

    syslog(LOG_WARNING, "Записываем PID!");

    ftruncate(fd, 0);
    sprintf(buf, "%ld", (long)getpid());
    write(fd, buf, strlen(buf) + 1);

    syslog(LOG_WARNING, "PID был записан!");

    return 0;
}


void daemonize(const char *cmd)
{
    int fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;

    /* Шаг 1. Сбросить маску режима создания файлов. */
    umask(0);


    //Получить максимально возможный номер дескриптора.
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
        perror("Невозможно получить максимальный номер дискриптора!\n");
    }


    /* Шаг 2. Вызвать функцию fork и завершить родительский процесс. */
    if ((pid = fork()) < 0) {
        perror("Ошибка вызова функции fork!\n");
    }
    else if (pid != 0) {
        exit(0);
    }


    /* Шаг 3. Стать лидером новой сессии, чтобы утратить управляющий терминал. */
    setsid();

    
    //Обеспечить невозможность обретения управляющего терминала в будущем.
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGHUP, &sa, NULL) < 0) {
        perror("Невозможно игнорировать сигнал SIGHUP!\n");
    }


    /* Шаг 4. Назначить корневой каталог текущим рабочим каталогом,
                чтобы впоследствии можно было отмонтировать файловую систему. */
    if (chdir("/") < 0) {
        perror("Невозможно назначить корневой каталог текущим рабочим каталогом!\n");
    }


    /* Шаг 5. Зактрыть все открытые файловые дескрипторы. */
    if (rl.rlim_max == RLIM_INFINITY) {
        rl.rlim_max = 1024;
    }

    for (int i = 0; i < rl.rlim_max; i++) {
        close(i);
    }


    /* Шаг 6. Присоединить файловые дескрипторы 0, 1, 2 к /dev/null. */
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);


    //Инициализировать файл журнала.
    openlog(cmd, LOG_CONS, LOG_DAEMON);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
        syslog(LOG_ERR, "Ошибочные файловые дескрипторы %d %d %d\n", fd0, fd1, fd2);
        exit(1);
    }

    syslog(LOG_WARNING, "Демон запущен!");
}


void handlerSIGHUP(int signum)
{
    long int ttime = time(NULL);
    syslog(LOG_WARNING, "Получен сигнал SIGHUP.\n User: %s. Time: %s", getlogin(), ctime(&ttime));
}


void thr_fn(void *arg)
{
    int err, signo;
    
    for (;;) {
        err = sigwait(&mask, &signo);
        
        if (err != 0) {
            syslog(LOG_ERR, "Ошибка вызова функции sigwait!");
            exit(1);
        }

        switch (signo) {
            
        case SIGHUP:
            handlerSIGHUP(signo);
            break;

        case SIGTERM:
            syslog(LOG_INFO, "Получен сигнал SIGTERM. Выход.");
            exit(0);
            break;

        default:
            syslog(LOG_INFO, "Получен непредвиденный сигнал %d!\n", signo);
            break;
        }
    }
}


int main()
{
    int err;
    pthread_t tid;
    struct sigaction sa;

    daemonize("newDaemon");
    

    /* Проверка, что ранее не была запущена другая копия демона. */
    if (already_running()) {
        syslog(LOG_ERR, "Демон уже запущен!\n");
        exit(1);
    }


    /* Восстановить действие по умолчанию для сигнала SIGHUP. */
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    if (sigaction(SIGHUP, &sa, NULL) < 0) {
        perror("Невозможно восстановить действие для SIGHUP!\n");
    }

    sigfillset(&mask);
    
    if ((err = pthread_sigmask(SIG_BLOCK, &mask, NULL)) != 0) {
        perror("Ошибка выполнения операции SIG_BLOCK!\n");
        exit(1);
    }


    /* Создать поток */
    err = pthread_create(&tid, NULL, thr_fn, 0);
    if (err != 0) {
        perror("Невозможно создать поток!\n");
        exit(1);
    }

    while (1) {
        sleep(5);
    }

    return 0;
}
