make clean
rm -fv record-mode-llama-cli replay-mode-llama-cli
make GGML_HIPBLAS=1 -j12 llama-cli