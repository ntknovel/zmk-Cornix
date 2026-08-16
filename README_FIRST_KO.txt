Cornix STENO v1.7.0 canonical dictionary — 빠른 시작
====================================================

1. 기존 Cornix 저장소 루트에 UPLOAD ZIP을 덮어씁니다.
2. 다음 검증을 실행합니다.

   python3 tools/validate_package.py .
   bash validation/run_host_tests.sh

3. 커밋·푸시합니다.

   git add -A
   git commit -m "Update Cornix STENO to v1.7.0 canonical dictionary"
   git push

4. GitHub Actions에서 left/right/settings_reset 작업을 확인합니다.
5. 왼쪽과 오른쪽 UF2는 서로 구분해 플래시합니다.

v1.7.0 핵심:
- 완성 사전 기본 활성 42개
- direct exact mask 지원
- ABBR_L/R 서로 구분
- AB2만 좌우 VEXT 미러
- GUI 편집형 quick_0~quick_11 유지
