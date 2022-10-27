/* 비트 연산자만 사용해서 코딩
* 과제 2. unsigned short (16bit) 변수의 각 비트를 컨트롤 하기

        - unsigned short 변수 = 0 으로 초기값 가짐.
        - 키보드로 1 ~ 16의 비트 자리 입력을 받음
        - 1 / 0  을 사용자로부터 받아서 지정된 자리의 비트를 0 또는 1로 바꿔줌.

        - 다른 위치에 입력된 기존 데이터는 보존이 되어야 함
*/

#include <stdio.h>

int main() {
    unsigned short var = 0;
        int n;
        int flag;
    while (true)    {
        
        printf("비트위치 : ");
        scanf("%d", &n);


        printf("OFF/ON [0,1] : ");
        scanf("%d", &flag);
        

        if (1 <= n && n <= 16)
        {
            --n;
            if (flag == 1) {
                var = var | (1 << n);
            }
            else if (flag == 0) {
                var = var & ~(1 << n);
            }

            // 출력구문


            for (int i = 15; i >= 0; i--) {

                if (var >> i & 1) {
                    printf("%d 번 Bit : ON\n", i + 1);
                }
                else {
                    printf("%d 번 Bit : OFF\n", i + 1);
                }

            }
        }
        else {
            printf("비트 범위를 초과하였습니다.");
            break;
        }
    } 
    return 0;
}