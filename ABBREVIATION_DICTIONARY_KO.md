# Cornix STENO canonical exact-mask 약어 43개

Cornix v1.8.0 컴파일 기본값입니다. 퀵 Macro 12개는 별도 ZMK Macro 경로를 사용합니다.

| 분류 | 출력 | 논리 역할 | 상태 | 메모 |
|---|---|---|---|---|
| 기본 | 점점 | `I_J + F_J` | confirmed | 일반 WORD: 초ㅈ+종ㅈ |
| 기본 | 진짜 | `I_J + F_J + F_DOUBLE` | confirmed | 두 번째 ㅈ이 ㅉ임을 쌍종으로 표시 |
| 기본 | 결국 | `I_G + F_G` | confirmed |  |
| 기본 | 그렇게 | `I_G + I_R + F_G` | confirmed |  |
| 기본 | 그렇군 | `I_G + F_R + F_G` | confirmed |  |
| 기본 | 그런 | `I_G + F_R` | confirmed |  |
| 기본 | 그럼 | `ABBR_L + I_G + F_R` | confirmed |  |
| 기본 | 그래도 | `I_G + I_R + F_D` | confirmed |  |
| 기본 | 그런데 | `ABBR_L + I_G + I_R + F_D` | confirmed |  |
| 기본 | 그럼에도 | `I_G + I_R + I_NG + F_D` | confirmed |  |
| 기본 | 그대로 | `I_G + I_D + F_R` | confirmed |  |
| 기본 | 갑자기 | `I_G + F_J + I_DOUBLE` | confirmed | 마지막 반복 ㄱ을 쌍초/반복 표식으로 표시 |
| 기본 | 완전히 | `I_NG + I_J + F_H` | confirmed |  |
| 기본 | 조용히 | `I_NG + F_J + F_H` | confirmed | 현재 배열에서 초ㅈ+종ㅇ+종ㅎ은 오른손 검지 충돌. 초ㅇ+종ㅈ+종ㅎ으로 배치. |
| 기본 | 되었 | `I_D + I_NG` | confirmed | TAIL 조립용 일반 WORD 조각 |
| 기본 | 근데 | `I_G + F_D` | confirmed |  |
| 기본 | 그때 | `I_G + F_D + F_DOUBLE` | confirmed | 일반 WORD: 두 번째 ㄷ이 ㄸ임을 쌍종으로 표시 |
| 접속 | 게다가 | `I_G + I_D + I_DOUBLE` | confirmed |  |
| 접속 | 그리고 | `I_G + I_R + I_DOUBLE` | confirmed |  |
| 접속 | 그러자 | `I_G + I_R + F_J` | confirmed |  |
| 접속 | 그러기 | `ABBR_L + I_G + I_R + I_DOUBLE` | confirmed |  |
| 접속 | 그렇지 | `ABBR_L + I_G + I_R + F_J` | confirmed |  |
| 접속 | 하지만 | `I_H + I_J + F_M` | confirmed |  |
| 어미 | 였다 | `F_NG + F_D` | confirmed | ㅇㄷ군 최우선, 오른손-only |
| 어미 | 없다 | `ABBR_R + F_NG + F_D` | confirmed | ㅇㄷ군 2순위, 오른손 중심 |
| 어미 | 었다 | `ABBR_L + F_NG + F_D` | current |  |
| 어미 | 았다 | `ABBR_L + ABBR_R + F_NG + F_D` | current |  |
| 어미 | 이었다 | `F_DOUBLE + F_NG + F_D` | confirmed | 사용자 요청: 오른쪽 쌍종 + 종ㅇ + 종ㄷ; H/J 동일 검지 예외 |
| 어미 | 였었다 | `VEXT_R + F_NG + F_D` | confirmed | 사용자 요청: 오른쪽 쌍모 전용 exact mask; 일반 AB2 정규화 예외 |
| 어미 | 었었다 | `VEXT_L + ABBR_R + F_NG + F_D` | current |  |
| 어미 | 있다 | `VEXT_L + ABBR_L + F_NG + F_D` | current |  |
| 어미 | 에다 | `ABBR_R + I_NG + F_D` | current | 오른손 ㅇㄷ의 7개 무충돌 슬롯을 다 쓴 뒤 사용하는 혼합 예외; 사용자 제안 'ㅇ초성+ㄷ종성+약어'. |
| 어미 | 했다 | `F_H + F_D` | confirmed |  |
| 어미 | 됐다 | `F_DOUBLE + F_D` | confirmed | 쌍종=반복 ㄷ |
| 어미 | 는데 | `VEXT_L + F_N` | confirmed | 종ㄴ+종ㄷ이 같은 오른손 중지라 AB2 단일 코어 사용 |
| 어미 | 을지도 | `F_NG + F_J + F_D` | confirmed |  |
| 어미 | 어졌다 | `ABBR_R + F_NG + F_J + F_D` | confirmed |  |
| 어미 | 지만 | `F_J + F_M` | confirmed |  |
| 어미 | 습니다 | `VEXT_L + F_S + F_D` | confirmed | 겠/했/였/있/없/않/되었 + 습니다 조립 |
| 어미 | 었으면 | `VEXT_L + ABBR_R + F_M` | current | 되+었으면 등 조립 |
| 어미 | 하는게 | `F_H + F_N + F_G` | confirmed |  |
| 어미 | 오는게 | `F_NG + F_N + F_G` | confirmed |  |
| 어미 | 가는게 | `F_DOUBLE + F_N + F_G` | confirmed | 쌍종=반복 ㄱ |

## 어미 ㅇㄷ군

```text
F_NG + F_D                         → 였다
ABBR_L + F_NG + F_D                → 었다
ABBR_R + F_NG + F_D                → 없다
ABBR_L + ABBR_R + F_NG + F_D       → 았다
VEXT_R + F_NG + F_D                → 였었다
F_DOUBLE + F_NG + F_D              → 이었다
VEXT_L/R + ABBR_R + F_NG + F_D     → 었었다
VEXT_L/R + ABBR_L + F_NG + F_D     → 있다
```

일반 AB2만 VEXT 좌우 정규화를 사용하며, `VEXT_R+F_NG+F_D`는 였었다 전용 exact mask입니다.
