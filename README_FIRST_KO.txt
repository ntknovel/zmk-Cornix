CORNIX STENO Studio v2.2.9 — 전체 소스 ZIP (UF2 미포함)

1. 기존 GitHub ZMK 자동 빌더 v1.1.0에서 START.cmd를 실행합니다.
2. 이번 CORNIX_STENO_STUDIO_v2.2.9_Source.zip을 파일 선택합니다.
3. 저장소 선택 / 토큰 입력 / 빌드 버튼을 사용합니다.
4. 실제 빌드 성공 뒤 left/right UF2를 각각 해당 유닛에 기록합니다.

수정된 속기 48:
- 단독 500ms 미만 누르고 떼면: 순수 RAlt를 한 번 전송합니다.
- 500ms 이상 홀드하거나 누른 채 다른 키를 누르면: 기존 SELECT SHIFT.
- 선택 동작을 한 뒤 떼면: RShift/Select Nav만 해제; RAlt 전송 없음.
- BASE 48은 기존 200ms RAlt 탭 / RCtrl 홀드 그대로입니다.

변경표: KEYMAP_UPDATE_v2_2_9_KO.md
전체 배열표: KEYMAPS_v2_2_9.html
검사: bash validation/run_all.sh

Studio에 저장된 이전 키맵이 있으면 새 소스 기본값보다 우선할 수 있습니다.
개인 변경을 기록한 뒤 Restore Stock Settings로 이번 기본값을 불러오세요.
settings_reset은 설정/페어링 초기화용이며 이번 변경만을 위해 먼저 넣지 않습니다.
ARM/ZMK 최종 컴파일 및 실제 USB/키보드 타건은 별도 확인이 필요합니다.
