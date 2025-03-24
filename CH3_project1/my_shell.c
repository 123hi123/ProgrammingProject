#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_CMD_LENGTH 1024
#define MAX_ARGS 64
#define HISTORY_SIZE 10

char *history[HISTORY_SIZE];
int history_index = 0;

void execute_command(char *cmd) {
    char *args[MAX_ARGS];
    char *cmd1[MAX_ARGS], *cmd2[MAX_ARGS];
    char *token = strtok(cmd, " \t\r\n\a");
    int i = 0, pipe_index = -1;
    int input_redirect = -1, output_redirect = -1;
    char *input_file = NULL, *output_file = NULL;

    // 解析命令，处理管道和重定向
    while (token != NULL) {
        if (strcmp(token, "|") == 0) {
            pipe_index = i;
            args[i] = NULL;  // 截断 args
        } else if (strcmp(token, "<") == 0) {
            input_redirect = i;
        } else if (strcmp(token, ">") == 0) {
            output_redirect = i;
        } else {
            args[i] = token;
        }
        token = strtok(NULL, " \t\r\n\a");
        i++;
    }
    args[i] = NULL;

    // 处理输入重定向
    if (input_redirect != -1) {
        input_file = args[input_redirect + 1];
        args[input_redirect] = NULL;
    }

    // 处理输出重定向
    if (output_redirect != -1) {
        output_file = args[output_redirect + 1];
        args[output_redirect] = NULL;
    }

    if (args[0] == NULL) return;  // 空命令

    // 如果有 `|`，分割两条命令
    if (pipe_index != -1) {
        int j;
        for (j = 0; j < pipe_index; j++)
            cmd1[j] = args[j];
        cmd1[j] = NULL;  // 第一条命令结束

        int k = 0;
        for (j = pipe_index + 1; args[j] != NULL; j++, k++)
            cmd2[k] = args[j];
        cmd2[k] = NULL;  // 第二条命令结束

        int pipe_fd[2];
        if (pipe(pipe_fd) == -1) {
            perror("pipe failed");
            return;
        }

        pid_t pid1 = fork();
        if (pid1 == 0) {  // 第一个子进程
            if (input_file) {
                int fd_in = open(input_file, O_RDONLY);
                if (fd_in == -1) {
                    perror("input file open failed");
                    exit(1);
                }
                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
            }
            dup2(pipe_fd[1], STDOUT_FILENO);
            close(pipe_fd[0]);
            close(pipe_fd[1]);
            execvp(cmd1[0], cmd1);
            perror("exec failed");
            exit(127);
        }

        pid_t pid2 = fork();
        if (pid2 == 0) {  // 第二个子进程
            if (output_file) {
                int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd_out == -1) {
                    perror("output file open failed");
                    exit(1);
                }
                dup2(fd_out, STDOUT_FILENO);
                close(fd_out);
            }
            dup2(pipe_fd[0], STDIN_FILENO);
            close(pipe_fd[0]);
            close(pipe_fd[1]);
            execvp(cmd2[0], cmd2);
            perror("exec failed");
            exit(127);
        }

        close(pipe_fd[0]);
        close(pipe_fd[1]);
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
        return;
    }

    // 没有 `|`，直接执行命令
    pid_t pid = fork();
    if (pid == 0) {
        if (input_file) {
            int fd_in = open(input_file, O_RDONLY);
            if (fd_in == -1) {
                perror("input file open failed");
                exit(1);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }

        if (output_file) {
            int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out == -1) {
                perror("output file open failed");
                exit(1);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        execvp(args[0], args);
        perror("exec failed");
        exit(127);
    } else {
        waitpid(pid, NULL, 0);
    }
}

void add_to_history(char *cmd) {
    if (history_index < HISTORY_SIZE) {
        history[history_index++] = strdup(cmd);
    } else {
        free(history[0]);  // 释放最旧的记录
        for (int i = 1; i < HISTORY_SIZE; i++) {
            history[i - 1] = history[i];
        }
        history[HISTORY_SIZE - 1] = strdup(cmd);
    }
}

void print_history() {
    for (int i = 0; i < history_index; i++) {
        printf("%d %s\n", i + 1, history[i]);
    }
}

void free_history() {
    for (int i = 0; i < history_index; i++) {
        free(history[i]);
    }
}

int main() {
    char cmd[MAX_CMD_LENGTH];

    while (1) {
        printf("my_shell> ");
        fflush(stdout); // 确保提示符马上显示

        if (fgets(cmd, sizeof(cmd), stdin) == NULL) {
            if (feof(stdin)) { // 如果是 EOF，则清除错误标志，继续执行 shell
                clearerr(stdin);
                continue;
            }
            break;
        }

        cmd[strcspn(cmd, "\n")] = '\0'; // 移除换行符

        if (strcmp(cmd, "exit") == 0) {
            break;
        }

        if (strcmp(cmd, "history") == 0) {
            print_history();
            continue;
        }

        add_to_history(cmd);
        execute_command(cmd);
    }

    free_history();
    return 0;
}
