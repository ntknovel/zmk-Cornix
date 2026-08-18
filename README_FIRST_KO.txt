Cornix STENO v1.8.0 FULL PARITY — 빠른 시작
================================================

이 버전은 v1.7.1의 canonical 약어·어미 재배치를 유지하면서, 이전 분기에서 빠졌던
키업/약어/좌우이동/수정모드/LED 패치와 KLOR에서 확정한 조작 규칙을 다시 통합한 판입니다.

가장 안전한 적용 방법
----------------------
1. 현재 정상 저장소에서 새 브랜치를 만듭니다.

   cd /workspaces/zmk-Cornix
   git status
   git switch -c cornix-steno-v1.8.0-full-parity

2. Cornix_ZMK_STENO_FULL_PARITY_v1.8.0_UPLOAD.zip을 저장소 루트에 올린 뒤 덮어씁니다.

   unzip -o Cornix_ZMK_STENO_FULL_PARITY_v1.8.0_UPLOAD.zip -d .
   rm Cornix_ZMK_STENO_FULL_PARITY_v1.8.0_UPLOAD.zip

3. 검증합니다.

   python3 tools/validate_package.py .
   bash validation/run_host_tests.sh

4. 커밋하고 푸시합니다.

   git add -A
   git commit -m "Update Cornix STENO to v1.8.0 full parity"
   git push -u origin cornix-steno-v1.8.0-full-parity

5. GitHub Actions의 세 산출물이 모두 성공했는지 확인합니다.

   cornix_left_maxrange_steno_indicator
   cornix_right_maxrange_steno_indicator
   cornix_settings_reset

6. 왼쪽과 오른쪽 UF2는 서로 구분해 플래시합니다. settings_reset은 페어링을 실제로
   초기화해야 할 때만 사용합니다.

핵심 조작
---------
- 약어L 홀드 + 쌍초 반복 탭: 왼쪽 이동
- 약어R 홀드 + 쌍종 반복 탭: 오른쪽 이동
- 기호L 홀드 + 쌍초 반복 탭: Shift+왼쪽 선택
- 기호R 홀드 + 쌍종 반복 탭: Shift+오른쪽 선택
- 기호 홀드 + 자음/모음 순차 탭: 직접 자모 수정 모드
- ㅣ 홀드 + 물리 I/J/K/L: 위/왼쪽/아래/오른쪽
- 왼쪽 인코더: Shift+위/아래, 오른쪽 인코더: 위/아래
- STENO에서 오른쪽 인코더 클릭: RAlt 후 BASE형 영문 상태
- 영문 복귀 상태에서 오른쪽 인코더 또는 왼쪽 STENO 클릭: RAlt 후 STENO 복귀

약어
----
- canonical exact-mask 43개
- GUI에서 Text를 바꿀 수 있는 Quick Macro 12개
- 약어L/약어R은 서로 다른 선택자
- 였었다: 오른겹모+종ㅇ+종ㄷ
- 이었다: 오른쌍종+종ㅇ+종ㄷ
- 일반 50ms 오타 복구, 11개 겹받침 90ms 롤링 유지
