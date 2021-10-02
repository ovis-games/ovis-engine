FROM emscripten/emsdk:2.0.31

COPY . /ovis-engine
WORKDIR /ovis-engine/build
RUN emcmake cmake -DCMAKE_INSTALL_PREFIX=/ovis-engine/install ..
RUN emmake make -j
RUN emmake make install

FROM tianon/true
WORKDIR /ovis-engine
COPY --from=0 /ovis-engine/install/* ./
VOLUME /ovis-engine
