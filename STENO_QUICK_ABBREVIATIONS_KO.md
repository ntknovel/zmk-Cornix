# Cornix GUI 편집형 퀵 약어 M0~M11

## 구조

- **입력 조건:** Cornix STENO 엔진의 `약어L/R + 기본 모음 1개` exact-mask
- **출력 내용:** `config/cornix.keymap`의 일반 `zmk,behavior-macro` 노드 `quick_0`~`quick_11`
- **판정 시점:** 해당 STENO 스트로크의 마지막 Key-up

Macro 안에는 약어키를 넣지 않는다. 입력 조건은 이미 펌웨어에 고정되어 있고, Macro에는 출력할 키 입력만 둔다.

## 현재 12개 프리셋

```text
M0  quick_0   약어L+ㅡ  사구구
M1  quick_1   약어L+ㅗ  구서룡
M2  quick_2   약어L+ㅜ  피피쿵
M3  quick_3   약어R+ㅏ  스타라이트
M4  quick_4   약어R+ㅣ  유안나
M5  quick_5   약어R+ㅓ  유혜나
M6  quick_6   약어L+ㅏ  실험체
M7  quick_7   약어L+ㅓ  A 시
M8  quick_8   약어L+ㅣ  W 시
M9  quick_9   약어R+ㅗ  케이지
M10 quick_10  약어R+ㅜ  일렉트리스
M11 quick_11  약어R+ㅡ  마이스터
```

## Keymap Editor 수정

Macros 메뉴에서 `quick_0`~`quick_11`을 선택하고 출력용 Key Press만 수정한다. 노드 이름은 C dispatch가 사용하는 고정 ID이므로 바꾸지 않는다.

## 텍스트 수정

한국어를 키코드로 직접 바꾸기 번거로우면 `config/steno_quick_text.tsv`의 출력 열만 고친 뒤 실행한다.

```bash
python3 tools/update_quick_macro_text.py
```

- 한글: 자동 두벌식 변환
- 영문·숫자 구간: RAlt로 영문 전환 후 출력하고 다시 한국어로 복귀
- 공백·기호: 그대로 HID 키로 변환
- 기본 Macro 속도: `wait-ms=40`, `tap-ms=40`

이 스크립트를 다시 실행하면 Keymap Editor에서 직접 편집한 12개 퀵 Macro는 TSV 값으로 다시 생성된다.
