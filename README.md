# Text Corrector service

## Build

```bash
mkdir build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j $(nproc)

# tc/cmd/string_bktree_builder for build dict
# tc/cmd/string_bktree_searcher for test dict
# tc/service/tc_service for restful service
```

## Dump Dict

```bash
cat TEXT_FILE | string_bktree_builder > DICT_FILE 
```

* each line in the TEXT_FILE will be indexed in DICT_FILE

## Serve

```bash
./tc_service --port=8080 --dicts=DICT_NAME:DICT_PATH,DICT_NAME:DICT_PATH,...
```
