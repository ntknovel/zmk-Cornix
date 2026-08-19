# STENO 독립 감사 수정 검증서 — 2026-08-19

## 수정한 차단 문제

1. `약어R+종ㅇ+종ㄷ`이 Right로 소모되어 `없다`가 나오지 않던 문제
2. 약어 selector 뒤 전용쌍초를 먼저 tap하면 `그러기`가 Left로 소모되던 문제
3. ㅋ/ㅇ 및 종ㅋ/종ㅇ 쌍자음 alias가 canonical 사전 조회에 연결되지 않던 문제
4. 오른손 쌍종에서 ㅃ·ㅉ·ㄸ이 빠져 있던 문제
5. 짧은 canonical 어미와 80ms 독립 쌍자모의 우선순위가 섞이던 문제
6. CORNIX 복합모음이 50ms보다 큰 release 차에서 한 키로 축약되던 문제
7. CORNIX 숫자·Undo·Redo 뒤 LED 하나가 남던 문제
8. 검증 코드가 실제 물리 role position과 다른 위치를 사용하던 문제
9. 배열 변경 뒤 약어 논리 mask가 바뀌어도 검출하지 못하던 문제
10. 내부 SHA-256 목록이 최신 파일과 맞지 않던 문제

## 새 판정 순서

- 사전: raw exact → 쌍자음 alias 정규화 → ABBR 반대쪽 조건부 fallback
- 직접입력: 모든 역할이 80ms 이상 유지된 유효 모음/쌍자음 chord를 첫 release에서 latch
- 짧은 일반 stroke: canonical 약어 및 일반 음절 우선
- 이동: 전용 쌍초/쌍종만 사용; ㅋ/ㅇ alias는 이동 trigger에서 제외
- LED: 모든 물리 press/release가 engine consume 여부와 독립적으로 set/clear

## 독립 회귀군

- 42 역할 single/pair/triple: 12,383
- canonical 약어 43 × 입력순서 4: 172
- 복합·확장 모음 19 × release순서 2, 55ms 차: 38
- 쌍자음 selector 3 × target 5 × 좌우 × release순서 2: 60
- 단독 자음 28 + 기본 모음 6: 34
- 79/80ms 경계와 알려진 실패 순서 특수시험

CORNIX와 KLOR 모두 위 회귀군을 통과해야 패키지 검증이 성공한다.
