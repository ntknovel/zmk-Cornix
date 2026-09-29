# ZMK Keymap Editor에서 Cornix 퀵 약어를 수정하는 방법

## STENO 키가 Unknown으로 보이는 이유

Cornix STENO 키는 표준 `&kp`가 아니라 외부 커스텀 behavior다. Keymap Editor가 해당 schema를 모르면 Unknown으로 보일 수 있다. STENO 레이어의 Unknown 바인딩은 건드리지 않아도 펌웨어에서는 정상 역할 ID로 사용된다.

## Macros 화면에서 찾을 항목

```text
quick_0  M0 Quick L+EU
quick_1  M1 Quick L+O
quick_2  M2 Quick L+U
quick_3  M3 Quick R+A
quick_4  M4 Quick R+I
quick_5  M5 Quick R+EO
quick_6  M6 Quick L+A
quick_7  M7 Quick L+EO
quick_8  M8 Quick L+I
quick_9  M9 Quick R+O
quick_10 M10 Quick R+U
quick_11 M11 Quick R+EU
```

## 편집 순서

1. 저장소를 Keymap Editor에서 연다.
2. **Macros** 메뉴를 연다.
3. 수정할 `quick_#` Macro를 고른다.
4. 실제 출력할 Key Press 시퀀스를 수정한다.
5. `quick_#` 이름은 바꾸지 않는다.
6. 저장·커밋하고 GitHub Actions에서 다시 빌드한다.

약어L/R이나 모음 역할을 Macro에 넣지 않는다. 예를 들어 `quick_1`의 입력 조건인 `약어L+ㅗ`는 STENO 엔진에 이미 고정되어 있다.

한글 키 시퀀스 편집이 번거로우면 `config/steno_quick_text.tsv`와 `tools/update_quick_macro_text.py`를 사용하는 편이 빠르다.
