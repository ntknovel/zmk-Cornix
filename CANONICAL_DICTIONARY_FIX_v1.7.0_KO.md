# Cornix STENO v1.7.0 canonical dictionary fix

## 증상

- `겹R+약R = '` 같은 예약 chord는 정상
- quick_0~quick_11도 정상
- 기본·접속·어미 사전 약어는 전부 무출력

## 원인

이전 61개 목록은 모든 단어에 강제로 bank 1/2를 붙인 초기 시제품이었습니다. 실제 완성
사전의 direct exact mask와 ABBR_L/R 구분이 런타임 구조에 반영되지 않았습니다.

## 수정

- 완성 사전의 기본 활성 42개로 교체
- full logical role mask lookup
- direct consonant exact mask 지원
- ABBR_L/R 구분
- AB2만 VEXT 좌우 정규화
- 직접 긴 약어가 50ms 보정으로 짧은 약어가 되지 않도록 exact-mask anchor 추가
- quick macro 12개는 기존 ZMK Macro 경로 유지
