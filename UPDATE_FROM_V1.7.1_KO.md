# v1.7.1 → v1.8.0 적용

정확한 v1.7.1 소스라면 작은 패치를 사용할 수 있지만, 여러 버전의 ZIP을 덮어쓴 이력이 있으면
`Cornix_ZMK_STENO_FULL_PARITY_v1.8.0_UPLOAD.zip`으로 저장소 전체를 덮어쓰는 방식을 권장한다.

```bash
cd /workspaces/zmk-Cornix
git status
git switch -c cornix-steno-v1.8.0-full-parity

unzip -o Cornix_ZMK_STENO_FULL_PARITY_v1.8.0_UPLOAD.zip -d .
rm Cornix_ZMK_STENO_FULL_PARITY_v1.8.0_UPLOAD.zip

python3 tools/validate_package.py .
bash validation/run_host_tests.sh

git add -A
git commit -m "Update Cornix STENO to v1.8.0 full parity"
git push -u origin cornix-steno-v1.8.0-full-parity
```
