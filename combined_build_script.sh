rm -rfv record-mode-llama-cli replay-mode-llama-cli
make clean
make GGML_HIPBLAS=1 -j12 CPPFLAGS="-UREPLAY_MODE" llama-cli
mv -v llama-cli record-mode-llama-cli
make clean
make GGML_HIPBLAS=1 -j12 CPPFLAGS="-DREPLAY_MODE" llama-cli
mv -v llama-cli replay-mode-llama-cli
make clean
