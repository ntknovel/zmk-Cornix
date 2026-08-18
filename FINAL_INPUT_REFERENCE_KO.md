# CORNIX / KLOR 한국어 STENO 최종 입력 기준 v2026-08-19

## 1. 자음 배열

```text
Q W E R T | Y U I O P
ㅂ ㅈ ㄷ ㄱ ㅅ | 종ㅅ 종ㄱ 종ㄷ 종ㅈ 종ㅂ

A S D F G | H J K L ;
ㅁ ㄴ ㅇ ㄹ ㅎ | 종ㅎ 종ㄹ 종ㅇ 종ㄴ 종ㅁ

Z X C V B | N M , . /
ㅋ ㅌ ㅊ ㅍ 쌍초 | 쌍종 종ㅍ 종ㅊ 종ㅌ 종ㅋ
```

논리 역할 ID와 canonical 약어 mask는 기존 v1.8.0/V29.4 계보를 유지하고, 위 물리 위치로만 재배치한다. 같은 손가락에 놓인 약어 조합도 임의로 초성/종성을 교체하지 않는다.

## 2. 숫자

기호L 또는 기호R 중 어느 쪽이든 + 상단열 하나:

`Q W E R T Y U I O P -> 1 2 3 4 5 6 7 8 9 0`

옛 세로 2키 숫자 chord는 폐기한다. 기호+상단열은 숫자가 최우선이다.

## 3. ㅜ/Enter · ㅏ/Space Tap/Hold/Chord

- 단독 입력 < 150ms 후 release: ㅜ키=Enter, ㅏ키=Space
- 단독 입력 >= 150ms 후 release: 해당 키 자체의 자모를 직접 1회 입력
- 홀드 임계값을 넘겨도 누르고 있는 동안에는 출력하지 않고 release에서 확정
- 다른 STENO 역할이 한 번이라도 참여: 시간과 관계없이 모음 ㅜ/ㅏ 및 해당 STENO stroke로 판정
- key-up 순서가 달라도 다중키로 사용되었으면 단독 Enter/Space/직접자모 출력은 취소

## 4. 약어 L/R

- canonical 약어의 논리 자음 조합은 기존 확정 조합을 유지한다.
- 같은 core에 L/R 서로 다른 결과가 둘 다 존재하면 exact L/R을 구분한다.
- 한쪽 selector만 등록된 core는 약어L/약어R 어느 쪽으로도 그 약어를 입력할 수 있다.
- 양쪽 약어키 자체가 조건인 항목은 둘 다 필요하다.
- Quick Macro의 L/R 별도 슬롯은 exact side를 유지한다.

## 5. 겹모(VEXT)

- 일반 AB2/VEXT 입력은 좌우 겹모를 동일하게 허용한다.
- 명시적 방향 예외는 exact side를 유지한다.
- `였었다 = 오른겹모 + 종ㅇ + 종ㄷ`을 보존한다.

## 6. 겹받침 / 같은 손가락

현대 한글 11개 겹받침과 기존 final rolling/key-up 복구를 유지한다. 새 배열에서 같은 손가락이 되는 약어는 펌웨어가 다른 자음으로 치환하지 않는다.

## 7. KLOR 저장 데이터 migration

KLOR V30.1은 KSD format v8을 사용한다. V29.x 저장 데이터가 새 물리 배열에서 잘못 재해석되지 않도록 V30.1 signature에서 기본 role map/dictionary migration을 수행한다. Raw HID wire protocol은 v7을 유지한다.

## 8. 검증

- CORNIX: package validator + host tests PASS; role projection 12,383 cases PASS
- KLOR: 20-step validation PASS; role projection 12,383 cases PASS
- canonical dictionary: 43 entries + Quick Macro 12 slots


## v2.1.1 단일 홀드 직접자모 규칙

별도 EDIT 키나 기호키를 수정 modifier로 사용하지 않는다.

- 초성 역할키 단독 150ms 이상 홀드 후 release → 해당 자음 직접 1회
- 종성 역할키 단독 150ms 이상 홀드 후 release → 초성/종성 구분 없이 같은 자음 직접 1회
- 기본 모음 역할키 단독 150ms 이상 홀드 후 release → 해당 모음 직접 1회
- ㅜ·Enter / ㅏ·Space 키는 짧은 탭만 Enter/Space, 150ms 이상 단독 홀드-release는 각각 ㅜ/ㅏ 직접 입력
- 다른 STENO 역할이 한 번이라도 같은 스트로크에 참여하면 홀드 시간과 무관하게 일반 속기 조합으로 처리
- 쌍초/쌍종, ABBR, VEXT, SYMBOL처럼 단독 자모가 없는 역할은 기존 단독 기능을 유지
- SYMBOL은 숫자/기호/선택 기능용이며 더 이상 자모 correction modifier로 사용하지 않음
