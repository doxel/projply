FROM osgeo/proj
ARG user=user
RUN useradd --create-home --shell /bin/bash $user \
 && apt-get update && apt-get install git pkg-config \
 && apt-get clean && rm -rf /var/lib/apt/lists/*
COPY . /home/$user/src/projply
RUN chown $user.$user /home/$user -R
USER $user
WORKDIR /home/$user/src/projply
RUN mkdir -p build \
 && cd build \
 && cmake ../src \
 && make -j $(nproc)
ENV PATH=/home/$user/src/projply/build/:${PATH}
