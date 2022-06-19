#include <string>
#include <sys/stat.h>
using namespace std;
/*
	struct stat {
		dev_t     st_dev;         // 파일을 담은 장치의 ID
		ino_t     st_ino;         // 아이노드 번호
		mode_t    st_mode;        // 파일 종류 및 모드
		nlink_t   st_nlink;       // 하드 링크 수
		uid_t     st_uid;         // 소유자의 사용자 ID
		gid_t     st_gid;         // 소유자의 그룹 ID
		dev_t     st_rdev;        // 장치 ID (특수 파일인 경우)
		off_t     st_size;        // 총 크기, 바이트 단위
		blksize_t st_blksize;     // 파일 시스템 I/O의 블록 크기
		blkcnt_t  st_blocks;      // 할당된 512B 블록 수

		struct timespec st_atim;  // 최근 접근 시간
		struct timespec st_mtim;  // 최근 수정 시간
		struct timespec st_ctim;  // 최근 상태 변경 시간
	}

 */
void	checkFile(const string& path)
{
	struct	stat s;
	int		status = 0;
	bzero(&s, sizeof(struct stat));

	status = stat(path.c_str(), &s);
	if (status == -1)
	{
		switch (errno)
		{
		case ENOENT:
		case ENOTDIR:
		case ENAMETOOLONG:
			;//HTTP_NOT_FOUND
			break;

		case EACCES:
		case ELOOP:
            ;//HTTP_FORBIDDEN
			break;
		default:
			;//HTTP_INTERNAL_SERVER_ERROR
        }
	}
	if (S_ISDIR(s.st_mode))

}
