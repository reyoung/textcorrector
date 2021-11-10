FROM gcc:11-bullseye as builder
RUN apt update && apt install -y cmake
COPY . /src/
RUN cd /src/ && mkdir build &&\
    cd build && cmake -DCMAKE_BUILD_TYPE=Release .. &&\
    make -j $(nproc)

FROM gcc:11-bullseye
COPY --from=builder /src/build/tc/cmd/string_bktree_builder/string_bktree_builder /bin/string_bktree_builder
COPY --from=builder /src/build/tc/cmd/string_bktree_searcher/string_bktree_searcher /bin/string_bktree_searcher
COPY --from=builder /src/build/tc/service/tc_service /bin/tc_service
