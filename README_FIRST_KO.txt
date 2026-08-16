Cornix ZMK 한국어 STENO 퀵 약어 프리셋 v1.6.3
================================================

핵심 변경
---------
- 약어키/2차 겹모 은행을 홀드 앵커로 사용해 순차 탭 약어가 50ms 오타 보정으로 사라지는 문제 수정
- 61개 사전 약어와 quick_0~quick_11 모두 같은 앵커 경로로 안정화
- BASE 왼쪽 하단 [: 탭 [ / 홀드 Ctrl+Y
- BASE 오른쪽 하단 ]: 탭 ] / 홀드 RCtrl
- BASE 오른쪽 최상단: Ctrl+A
- 기존 4개 삼중모음 퀵 슬롯을 폐기하고, 완성 약어사전 규격의 M0~M11 12개 슬롯으로 교체
- 사용자가 지정한 12개 출력 문구를 일반 ZMK Macro에 미리 입력
- ZMK Keymap Editor의 Macros 화면에서 quick_0~quick_11을 바로 수정 가능
- 한국어 텍스트만 고쳐 재생성하는 config/steno_quick_text.tsv 방식도 추가
- v1.5.1의 오른손 peripheral/settings_reset 빌드 하드닝 유지

가장 쉬운 적용
-------------
1. 아직 v1.5.1을 적용하지 않았거나 저장소 상태가 섞였다면
   Cornix_ZMK_STENO_ABBR_ANCHOR_FIX_v1.6.3_UPLOAD.zip을 저장소 루트에 덮어씁니다.
2. 이미 v1.5.1이 적용된 상태라면
   Cornix_ZMK_STENO_ABBR_ANCHOR_FIX_v1.6.3_PATCH_ONLY.zip만 덮어씁니다.
3. Codespaces 터미널에서 실행합니다.

   cd /workspaces/zmk-Cornix
   python3 tools/validate_package.py .
   git add -A
   git commit -m "Fix Cornix STENO abbreviation anchor"
   git push

4. GitHub Actions에서 left/right/settings_reset 빌드가 모두 성공한 뒤 UF2를 사용합니다.

현재 퀵 출력
------------
약어L+ㅗ = 구서룡      약어L+ㅡ = 사구구      약어L+ㅜ = 피피쿵
약어R+ㅏ = 스타라이트  약어R+ㅣ = 유안나      약어R+ㅓ = 유혜나
약어R+ㅗ = 케이지      약어R+ㅡ = 마이스터    약어R+ㅜ = 일렉트리스
약어L+ㅏ = 실험체      약어L+ㅣ = W 시         약어L+ㅓ = A 시

빠른 수정
---------
방법 A: ZMK Keymap Editor -> Macros -> quick_0~quick_11의 Key Press 시퀀스 수정
방법 B: config/steno_quick_text.tsv의 3번째 열만 수정 후 아래 실행

   python3 tools/update_quick_macro_text.py
   python3 tools/validate_package.py .

Windows에서는 UPDATE_QUICK_MACROS.cmd를 실행해도 됩니다.
방법 B를 실행하면 quick_0~quick_11의 GUI 편집 내용은 TSV 값으로 다시 생성됩니다.
