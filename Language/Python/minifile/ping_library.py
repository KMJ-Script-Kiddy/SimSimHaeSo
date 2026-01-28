from ping3 import ping, verbose_ping
import time
import os

try:
    for i in range(3):
        target = ""
        response_time = ping(target, unit='ms')

        if response_time != 0:
            print(f"{target} 응답 없음 (Timeout)")
        else:
            print(f"{target} 응답 시간: {response_time:.2f} ms")
            time.sleep(1)

except KeyboardInterrupt:
    print(f"사용자 입력에 의해 작업이 취소되었습니다.")
