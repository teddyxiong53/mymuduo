#include "daemon.h"
#include "mylog.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <memory>

Daemon::Daemon(const std::string& user, const std::string& group, const std::string& pidFile)
 : m_pidFileHandle(-1),
   m_user(user),
   m_group(group),
   m_pidFile(pidFile)
{

}

Daemon::~Daemon()
{
    if(m_pidFileHandle != -1) {
        close(m_pidFileHandle);
    }
}

bool Daemon::daemonize()
{
    //首先是创建pidFile对应的目录，因为层级比较多。
    //所以需要要把父目录也创建。
    //用system创建，这样最简单。
    //先根据pidFile完整路径得到目录。
    std::string dir = m_pidFile.substr(0, m_pidFile.find_last_of('/'));
    std::string cmd = "mkdir -p ";
    cmd += dir;
    system(cmd.c_str());

    //然后看看目录创建是否成功。
    if(access(dir.c_str(), F_OK) != 0) {
        myloge("create pid dir fail");
        return false;
    }
    //然后创建对应的pid文件。
    m_pidFileHandle = open(m_pidFile.c_str(), O_RDWR|O_CREAT, 0644);
    if(m_pidFileHandle < 0) {
        myloge("create pid file fail");
        return false;
    }
    uid_t user_uid = -1;
    gid_t user_gid = -1;
    std::string user_name;
    if(!m_user.empty()) {
        //user指定了
        struct passwd *pwd = getpwnam(m_user.c_str());
        if(pwd == nullptr) {
            myloge("no such user");
            return false;
        }
        user_uid = pwd->pw_uid;
        user_gid = pwd->pw_gid;
        user_name = strdup(m_user.c_str());
        setenv("HOME", pwd->pw_dir, true);
    }
    if(!m_group.empty()) {
        //如果group也指定了
        struct group* grp = getgrnam(m_group.c_str());
        if(grp == nullptr) {
            myloge("no such group");
            return false;
        }
        user_gid = grp->gr_gid;
    }
    //把pidFile的owner改成设置的user和group
    if(chown(m_pidFile.c_str(), user_uid, user_gid) < 0) {
        myloge("chown fail");
        return false;
    }
    //设置uid
    if(
        (user_uid != -1)
        && user_uid != getuid()
        && setuid(user_uid) == -1
    )
    {
        myloge("setuid fail");
        return false;
    }
    pid_t pid, sid;
    pid = fork();
    if(pid < 0) {
        exit(1);
    }
    if(pid > 0) {
        //父进程退出
        exit(0);
    }
    //下面是子进程的处理
    umask(0);
    sid = setsid();
    if(sid < 0) {
        myloge("setsid fail");
        exit(1);
    }
    if(chdir("/") < 0) {
        myloge("chdir fail");
        exit(1);
    }
    //锁住文件
    if(lockf(m_pidFileHandle, F_TLOCK, 0)== -1) {
        myloge("lockf pid file fail");
        return false;
    }
    char str[10];
    sprintf(str, "%d\n", getpid());
    //把pid写入到文件里
    if(write(m_pidFileHandle, str, strlen(str)) != (int)strlen(str)) {
        myloge("write pid to file fail");
        return false;
    }
    //关闭stdin、stdout、stderr
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    return true;
}
