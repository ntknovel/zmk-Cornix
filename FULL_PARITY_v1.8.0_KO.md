# Cornix STENO v1.8.0 통합 패치 감사

| 계보/요구 | v1.8.0 상태 | 설명 |
|---|---|---|
| v1.6.1 BASE 키 수정 | 유지 | 기존 일반 키보드 레이어와 RF 키 유지 |
| v1.6.2 settings_reset 빌드 수정 | 유지 | STENO 모듈을 reset 산출물에서 명시적으로 비활성 |
| v1.6.3 약어 앵커 | 확장 통합 | 약어L/R뿐 아니라 canonical direct-prefix도 key-up 보호 |
| 초기 v1.7 key-up/약어/nav/LED | 복구·갱신 | 50ms 보정, ㅣ+JLKI, LED 카테고리 복구 |
| canonical dictionary v1.7.0 | 유지 | 43개 최종 exact-mask, 낡은 61/88개 bank 사전은 복원하지 않음 |
| v1.7.1 tail remap | 유지 | 였었다/이었다/었었다/있다 최종 배치 |
| Quick Macro 12개 | 유지 | GUI 편집형 ZMK macro와 기본 Text 유지 |
| KLOR식 좌우 이동 통일 | 적용 | 약어+쌍자음=이동, 기호+쌍자음=Shift 선택 |
| 연속 직접 수정 | 적용 | 기호 홀드 중 자음·모음 순차 출력 |
| 인코더 상하/선택 상하 | 적용 | 왼쪽 Shift+상하, 오른쪽 상하 |
| 영문 복귀 상태 | 적용 | STENO RAlt→BASE형 영문→RAlt+STENO 복귀 |
| 팬텀 역할 감사 | 적용 | 42역할 single/pair/triple 12,383조합 검사 |
| KLOR matrix active-discharge | 미적용 | Cornix는 ZMK 표준 GPIO matrix driver라 같은 수정 대상 아님 |
| KLOR Gothic OLED | 해당 없음 | Cornix는 OLED 대신 상태 LED 사용 |
| KLOR Vial KSD2 동적 사전 | 해당 없음 | Cornix는 ZMK compile-time canonical 사전 + GUI Quick Macro 구조 |

## 이동 최종 규칙

```text
약어L + 쌍초 반복 탭 → ←
약어R + 쌍종 반복 탭 → →
기호L + 쌍초 반복 탭 → Shift+←
기호R + 쌍종 반복 탭 → Shift+→
```

## 약어 우선순위

1. Quick Macro exact mask
2. 일반 STENO 기호·편집 exact mask
3. canonical dictionary exact mask
4. 일반 한글 음절
5. 미등록 조합은 무출력

`약어+쌍자음` 두 키만 정확히 사용하면 이동이지만, 더 큰 canonical 약어가 같은 두 역할을
포함하면 전체 exact mask가 우선된다.
