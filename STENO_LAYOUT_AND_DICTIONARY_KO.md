# Cornix STENO 최종 배열·사전 안내 v1.6.1

## 최종 자음 배열

```text
Q=ㅋ W=ㅈ E=ㄴ R=ㅎ T=ㅌ | Y=종ㅌ U=종ㅎ I=종ㄴ O=종ㅈ P=종ㅋ
A=ㅅ S=ㄱ D=ㄹ F=ㅇ G=쌍초 | H=쌍종 J=종ㅇ K=종ㄹ L=종ㄱ ;=종ㅅ
Z=ㅊ X=ㅂ C=ㄷ V=ㅁ B=ㅍ | N=종ㅍ M=종ㅁ ,=종ㄷ .=종ㅂ /=종ㅊ
```

## 완성 사전

- `config/steno_dictionary.tsv`
- 61개 exact-mask 약어
- 논리 역할 저장, 물리 배열과 분리

## 퀵 사전

- 고정 입력 마스크: `config/steno_quick_abbreviations.tsv`
- 사용자 텍스트 프리셋: `config/steno_quick_text.tsv`
- standard ZMK Macro: `quick_0`~`quick_11`
- 슬롯 수: 12

판정 우선순위에서 퀵 exact-mask를 먼저 소비하고, 일치하지 않으면 숫자·기호·일반 음절·은행 1/2 약어를 처리한다. 부분집합이나 가장 가까운 조합은 추정하지 않는다.
