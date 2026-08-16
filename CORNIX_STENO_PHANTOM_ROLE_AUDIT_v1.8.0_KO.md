# Cornix STENO 팬텀 역할 감사 — v1.8.0

## 결론

KLOR에서 확인된 커스텀 active-HIGH 스캐너의 짧은 settling 문제는 Cornix 저장소의 STENO
소프트웨어 경로에는 존재하지 않는다. Cornix는 ZMK 표준 GPIO matrix driver가
물리 position 이벤트를 만든 뒤, 이 모듈이 position 하나를 논리 role 하나로만 투영한다.

Cornix STENO 엔진은 다음 작업만 수행한다.

- 실제 position에 저장된 role을 최종 mask에 OR
- 50ms 보정에서 먼저 놓인 position을 제거
- 90ms 안의 합법 겹받침 첫 position을 복원
- exact dictionary mask 조회

입력하지 않은 제3 role을 새로 합성하는 경로는 없다.

## 전수 검증

`validation/test_phantom_role_invariants.py`가 다음을 검사한다.

- role-bearing 물리 위치 42개
- 42개 역할이 one-to-one으로 한 위치에만 연결
- 자음 30개 위치와 `steno_role_layout.tsv` 일치
- 42개 역할의 모든 single/pair/triple, 총 12,383조합 투영
- 입력 mask 바깥의 제3 역할이 내부 상태에 생기지 않음
- 합법 겹받침 pair가 정확히 11개
- 종ㄷ+종ㅂ 및 종ㄷ+종ㅂ+종ㅁ은 decoder에서 거부
- 모든 ZMK combo가 STENO 레이어에서 제외

v1.8.0의 이동·수정 스트림도 같은 불변성을 지킨다.

- 약어/기호 앵커는 실제 같은 쪽 쌍자음 탭만 이동으로 소비
- 직접 수정 모드는 실제 unit의 role mask만 decoder에 전달
- dictionary prefix 보존은 role을 추가하지 않고 삭제만 지연

## 남는 하드웨어 가능성

하드웨어 matrix driver가 실제로 제3 position 이벤트를 보고하면 상위 STENO 엔진은 그것을
진짜 입력으로 받는다. Cornix 실기에서 팬텀이 재현되면 특정 자음을 사후 삭제하지 말고 다음을
확인해야 한다.

1. ZMK event/position 로그
2. Matrix Tester 또는 Studio에서 제3 position이 실제로 켜지는지
3. 다이오드 방향·납땜·스위치·PCB 오염
4. 필요한 경우에만 ZMK matrix wait 설정의 하드웨어별 시험

증거 없이 Cornix의 기본 kscan timing은 변경하지 않는다.
