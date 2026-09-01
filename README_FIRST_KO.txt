CORNIX ZMK STENO v2.2.6 — 종성→초성 피로분산 미러 + 500ms 입력판정
=======================================================================

이 전체 저장소는 v2.2.5의 검증된 빌드 행과 v2.2.4의 빠른 일반 조합/500ms 단독
수정 구조를 유지하면서, 왼손 엄지 모음과 오른손 자음을 나누어 칠 수 있는 새 규칙을
추가한 권장 빌드본입니다.

새 피로분산 규칙
- 초성이 없을 때 정확히 `ㅗ`, `ㅡ`, `ㅜ` 중 하나와 오른손 종성 자음 1개를 누르면,
  그 종성 자음을 초성으로 사용합니다. 오른손 자음은 받침으로 다시 출력하지 않습니다.
- 예: `종ㅂ+ㅜ=부`, `종ㄱ+ㅗ=고`, `종ㅁ+ㅡ=므`
- 전용 쌍초 키(단독 출력 `,`) + `ㅗ/ㅡ/ㅜ` + 오른손 `종ㅂ/종ㅈ/종ㄷ/종ㄱ/종ㅅ`
  → 각각 `ㅃ/ㅉ/ㄸ/ㄲ/ㅆ` 초성
- 전용 쌍종 키(단독 출력 `.`)도 같은 오른손 미러 초성을 쌍자음으로 만들 수 있습니다.
- 반대로 전용 쌍종 키 + 왼손 `초ㅂ/초ㅈ/초ㄷ/초ㄱ/초ㅅ` + 모음은 왼손 초성을
  쌍자음으로 만듭니다.
- 요청 예시: `전용쌍초(,)+ㅜ+종ㅂ=뿌`, `전용쌍종(.)+초ㅂ+ㅏ=빠`
- 이 교차 쌍자음 규칙에는 전용 `,`/`.` 역할만 사용합니다. ㅋ/ㅇ 및 종ㅋ/종ㅇ은
  기존 동일 손 쌍자음 별칭으로는 유지되지만 새 교차 selector로는 사용하지 않습니다.
- 종성→초성 미러는 정확한 단모음 `ㅗ/ㅡ/ㅜ`에만 적용됩니다. `ㅏ/ㅣ/ㅓ`, 복합모음,
  종성 2개, 전용 `,`과 `.`의 동시 사용은 새 미러 경로로 해석하지 않습니다.

기존 입력 규칙 유지
- ㅜ 단독 500ms 미만: Enter / 500ms 이상 hold-release: ㅜ
- ㅓ 단독 500ms 미만: Space / 500ms 이상 hold-release: ㅓ
- 다른 STENO 키가 하나라도 참여하면 500ms 단독 후보를 즉시 취소하고 일반 조합으로 처리
- 모든 단독 자음·기본 모음·유효 복합모음·쌍자음 직접입력: 500ms 이상
- 전역 key-up 키 삭제 기능: OFF(0ms). 먼저 뗀 종ㄱ도 같은 stroke에 보존
- 마지막 키를 떼면 추가 대기 없이 즉시 확정
- 합법 겹받침 rolling은 90ms 국소 상태만 사용하며 일반 출력 확정을 지연하지 않음
- ㅂㅈㄷㄱㅅ 미러 배열, 약어 43개, Quick Macro 12개, 숫자/Undo/Redo/RF/LED 유지

빌드 구조
- 정상 좌/우와 settings_reset 행은 v2.2.5에서 교정한 pinned 구성을 그대로 유지
- settings_reset은 `studio-rpc-usb-uart nrf52840-nosd` snippets 사용
- 정상 left STENO 펌웨어의 ZMK Studio 활성화를 뜻하지 않음

로컬 소스 검증
  python3 tools/validate_package.py .
  bash validation/run_host_tests.sh

두 명령이 PASS한 뒤 저장소에 push하고 GitHub Actions의 세 작업을 확인합니다.
- cornix_left_maxrange_steno_indicator
- cornix_right_maxrange_steno_indicator
- cornix_settings_reset

세 작업이 모두 초록색이어야 생성 UF2를 사용합니다.
상세 규칙: V2.2.6_MIRRORED_ONSET_CROSS_DOUBLE_KO.md
전체 입력표: FINAL_INPUT_REFERENCE_KO.md
빌드 감사: V2.2.5_SETTINGS_RESET_MATRIX_FIX_KO.md
