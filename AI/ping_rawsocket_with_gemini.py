import os, sys
import socket, struct, select
import time

def calculate_checksum(source_string):
    countTo = (len(source_string) // 2) *2
    sum = 0
    count = 0

    # 2바이트씩 더하기
    while count < countTo:
        thisVal = source_string[count + 1] * 256 + source_string[count]
        sum = sum + thisVal
        sum = sum & 0xffffffff
        count = count + 2

    if countTo < len(source_string):
        sum = sum + source_string[len(source_string) - 1]
        sum = sum & 0xffffffff

    # Carry
    sum = (sum >> 16) + (sum & 0xffff)
    sum = sum + (sum >> 16)

    # 1의 보수
    answer = ~sum
    answer = answer & 0xffff
    answer = answer >> 8 | (answer << 8 & 0xff00)
    return answer

def raw_ping(host):
    # DNS lookup
    dest_addr = socket.gethostbyname(host)
    print(f"Target IP: {dest_addr}")

    # socket 생성, 관리자 권한 필요
    try:
        my_socket = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_ICMP)
    except PermissionError:
        print("권한이 없습니다.")
        return

    my_id = os.getpid() & 0xFFFF # 현재 실행중인 프로세스 ID를 식별자로 사용

    header = struct.pack("!BBHHH", 8, 0, 0, my_id, 1)
    data = struct.pack("d", time.time())

    my_checksum = calculate_checksum(header + data)

    header = struct.pack("!BBHHH", 8, 0, socket.htons(my_checksum), my_id, 1)
    packet = header + data

    my_socket.sendto(packet, (dest_addr, 1)) # 포트 아무거나 -> 1번
    send_time = time.time()

    while True:
        ready = select.select([my_socket], [], [], 1.0)
        if ready[0] == []: # 타임아웃
            print("요청 시간 만료(Timeout)")
            return

        time_received = time.time()
        rec_packet, addr = my_socket.recvfrom(1024)

        icmp_header = rec_packet[20:28]
        type, code, checksum, p_id, sequence = struct.unpack("!BBHHH", icmp_header)

        if p_id == my_id:
            bytes_sent = struct.calcsize("d")
            time_sent = struct.unpack("d", rec_packet[28:28 + bytes_sent])[0]
            delay = (time_received - time_sent) * 1000 # 밀리초 단위

            print(f"{dest_addr} 응답: bytes={len(rec_packet)} time={delay:.2f}ms TTL={rec_packet[8]}")
            return

        my_socket.close()

if __name__ == "__main__":
    target = "172.30.1.254"
    print(f"--- Raw Socket API Ping from Gemini: {target} ---")

    try:
        while True:
            raw_ping(target)
            time.sleep(1)
    except KeyboardInterrupt:
        print("사용자 입력에 의해 취소되었습니다.")
