# CORNIX STENO v2.2.6 재감사 검증서

## 보존한 기반

- v2.2.4: 독립 수정 500ms, 일반 chord 마지막 key-up 즉시 확정, 전역 key-up 역할 삭제 OFF
- v2.2.5: pinned ZMK/Cornix commit과 공식 `settings_reset` snippets를 사용하는 3행 빌드 matrix
- canonical 43개, Quick Macro 12개, 미러 배열, 숫자·Undo/Redo·LED·RF 구조

## v2.2.6 새 규칙

1. 초성 없이 정확한 `ㅗ/ㅡ/ㅜ`와 오른손 종성 자음 하나를 입력하면 오른손 자음을 초성으로 소비한다.
2. 전용 쌍초 역할(단독 `,`)은 반대쪽 오른손 `종ㅂ/종ㅈ/종ㄷ/종ㄱ/종ㅅ`을 쌍초성으로 만든다.
3. 전용 쌍종 역할(단독 `.`)은 오른손 미러 초성과 왼손 일반 초성 양쪽 모두를 쌍초성으로 만들 수 있다.
4. 새 교차 selector는 전용 `,`/`.`만 인정하고 ㅋ/ㅇ·종ㅋ/종ㅇ은 제외한다.
5. 오른손 엄지 모음, 복합·확장 모음, 오른손 자음 2개, 두 전용 selector 동시 사용은 미러 경로가 아니다.
6. 왼손 초성이 있고 `.`와 다른 오른손 종성이 함께 있으면 기존 종성 쌍자음 해석을 보존한다.

## 타이밍 불변성

새 입력은 일반 chord 경로를 사용한다. 500ms는 독립 수정에만 쓰며, 다른 역할이 참여하면 이미 500ms를 넘긴 역할도 즉시 chord에 합류한다. 전역 key-up 역할 삭제가 꺼져 있으므로 `,`, `.`, 모음, 자음 중 어느 키를 먼저 떼어도 완성 mask가 마지막 key-up까지 보존된다.

## 회귀시험

- 역할 single/pair/triple 12,383건
- canonical 약어 43개 × 입력순서 4종 = 172건
- 복합·확장 모음 38건
- 기존 쌍자음 60건
- 새 피로분산·교차 쌍자음 timing/release-order 189건 + 제외 경계
- 단독 자모 34건 및 499/500ms 경계
- `전용쌍초(,)+ㅜ+종ㅂ=뿌`
- `전용쌍종(.)+초ㅂ+ㅏ=빠`
- 500ms 이후 늦은 chord 합류
- 기존 `박` 조기 release/늦은 종성 추가
- YAML/API-shaped C compile/LED/peripheral source 검사
- reset matrix/snippets/commit pin 검사

## 남은 최종 확인

패키지 내부 검증은 nRF52840 cross compiler와 실제 키보드를 대체하지 않는다. 저장소에 올린 뒤 GitHub Actions의 left, right, settings_reset 세 작업이 모두 성공해야 실제 UF2 빌드가 최종 확인된다.
