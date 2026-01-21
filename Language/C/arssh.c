#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define CONF "/etc/ssh/sshd_config"
#define CONF_TMP "/tmp/sshd_config.tmp"
#define PERMIT "PermitRootLogin"
#define PERMIT_S "#PermitRootLogin"

void backup() {
	time_t t = time(NULL);
	struct tm *tm = gmtime(&t); // UTC 시간 참조

	char ts[32];
	if (strftime(ts, sizeof(ts), "%Y%m%d%H%M%S", tm) == 0) {
		snprintf(ts, sizeof(ts), "unknown"); // 시간 참조 실패시
	}

	char CONF_BAK[128];
	snprintf(CONF_BAK, sizeof(CONF_BAK), "/etc/ssh/sshd_config.bak_%s", ts); // 파일 이름 지정
	
	pid_t pid = fork();
	if (pid == 0) { // 백업 파일 생성
		execl("/bin/cp", "cp", CONF, CONF_BAK, NULL);
		perror("[오류] 백업 파일 생성에 실패하였습니다");
		exit(1);
	}

	int status;
	wait(&status);
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
		perror("[오류] 백업 파일 생성 중 오류가 발생하였습니다");
		exit(1);
	}

	printf(">> 백업 파일을 생성하였습니다: %s\n", CONF_BAK);
	fflush(stdout);
}

void change_auth() {
	FILE *src = fopen(CONF, "r");
	FILE *dst = fopen(CONF_TMP, "w");
	if (!src || !dst) {
		perror("[오류] 파일을 여는 중 오류가 발생하였습니다");
		exit(1);
	}

	char line[256];
	char input[8];

	do {
		printf("변경할 설정을 입력하세요. [yes|no|prohibit-password]\n: ");
		if (fgets(input, sizeof(input), stdin) == NULL) {
			continue;
		}
		
		input[strcspn(input, "\n")] = '\0';

	} while (strcmp(input, "yes") != 0 && strcmp(input, "no") != 0 && strcmp(input, "prohibit-password") != 0);

	while (fgets(line, sizeof(line), src)) { // src 파일을 줄 마다 읽으며 확인
		if (strncmp(line, PERMIT, strlen(PERMIT)) == 0 || strncmp(line, PERMIT_S, strlen(PERMIT_S)) == 0)
			continue; // PermitRootLogin 중복 제거
		else
			fputs(line, dst);
	}
	fprintf(dst, "%s %s\n", PERMIT, input);

	if (fclose(src) != 0) {
		perror("[오류] 원본 파일 닫기에 실패하였습니다");
		exit(1);
	}
	
	if (fclose(dst) != 0) {
		perror("[오류] 임시 파일 닫기에 실패하였습니다");
		exit(1);
	}
	
	if (rename(CONF_TMP, CONF) != 0) { // 원본 파일로 붙여넣기
		perror("[오류] 작성중인 파일을 원본으로 교체하는 데 실패하였습니다");
		exit(1);
	}
}

void restart() {
	pid_t pid = fork();
	if (pid == 0) {
		execl("/bin/systemctl", "systemctl", "restart", "sshd", NULL);
		perror("[오류] 데몬 재시작에 실패하였습니다");
		exit(1);
	}

	int status;
	wait(&status);
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
		perror("[오류] 데몬 재시작 중 오류가 발생하였습니다");
		exit(1);
	}

	pid = fork();
	if (pid == 0) {
		execl("/bin/systemctl", "systemctl", "status", "sshd", NULL);
		perror("[오류] 데몬 상태 확인에 실패하였습니다");
		exit(1);
	}

	wait(NULL);

	pid = fork();
	if (pid == 0) {
		execl("/bin/tail", "tail", CONF, NULL);
		perror("[오류] 파일 읽기에 실패하였습니다");
		exit(1);
	}

	wait(NULL);
}

int main() {
	if (geteuid() != 0) {
		fprintf(stderr, "[오류] 관리자 권한으로만 실행 가능합니다.\n");
		return 1;
	}

	int menu;
	printf("1. Root 원격접속 상태 변경(사용자 입력)\n");
	printf("2. 프로그램 종료\n");
	printf(">> 메뉴를 선택하세요: ");
	scanf("%d", &menu);

	int c; while ((c = getchar()) != '\n' && c != EOF);

	switch (menu) {
		case 1:
			backup();
			change_auth();
			restart();
			break;
		case 2:
			printf(">> 프로그램을 종료합니다.\n");
			break;
	}

	return 0;
}
