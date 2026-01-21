#!/bin/bash
#Written by KMJ

Running() {
	Text="PermitRootLogin"
	File="/etc/ssh/sshd_config"
	Now=""
	Status="$(grep $Text $File > /dev/null 2>&1)"

	if [[ $Status == "yes" ]]; then
		Now="[현재 상태] Root 원격 접속 허용중"
	else
		Now="[현재 상태] Root 원격 접속 불가능"
	fi

	echo -e "SSH 데몬 재시작 프로그램\n1. Root 원격 접속 불허\n2. Root 원격 접속 허용"
	echo $Now
	read -p "메뉴를 선택하세요: " Choice

	case $Choice in
		1)
			sed -i "s/$Text yes/$Text no/g" $File > /dev/null 2>&1
			grep $Text $File
			systemctl restart sshd
			systemctl status sshd | grep active
			echo "Root 원격 접속을 [불허]하였습니다."
			;;
		2)
			sed -i "s/$Text no/$Text yes/g" $File > /dev/null 2>&1
			grep $Text $File
			systemctl restart sshd
			systemctl status sshd | grep active
			echo "Root 원격 접속을 [허용]하였습니다."
			;;
		*)
			echo "잘못 선택하셨습니다."
			;;
	esac
}

Running
