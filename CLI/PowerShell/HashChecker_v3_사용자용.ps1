# 사용자 입력
$FileLoc = Read-Host ">> 파일의 경로를 입력하세요"
$FileExt = Read-Host ">> 파일의 확장자를 입력하세요"
$Algorithm = Read-Host ">> 검사를 원하는 알고리즘을 입력하세요. 알고리즘: MACTripleDES|RIPEMD160|MD5|SHA512|SHA384|SHA256|SHA1
"
$InputHash = Read-Host ">> 원본 파일의 해시를 입력하세요($Algorithm)"

# 파일
$File = "$FileLoc\*.$FileExt"

# 명령어 및 검사
$OriginHash = (Get-FileHash $File -Algorithm $Algorithm).Hash

# 검증
if ($OriginHash -eq $InputHash) {
    Write-Host "해시가 일치합니다. 검사가 완료되었습니다."
} elseif ($FileLoc -eq "") {
    Write-Host "경로를 입력하지 않아 오류가 발생하였습니다. 검사가 중지되었습니다."
} elseif ($OriginHash -ne $InputHash) {
    Write-Host "해시가 일치하지 않습니다. 파일의 무결성이 온전하지 않습니다."
} else {
    Write-Host "실행 중 오류가 발생하였습니다. 검사가 중지되었습니다."
}