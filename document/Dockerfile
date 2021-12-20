# use ubuntu as doxygen build image
FROM ubuntu AS doxygen
RUN apt-get update && apt-get install -y doxygen

FROM doxygen AS build
# make app 
RUN mkdir /app
RUN mkdir /build

WORKDIR /build

#copy doxyfile to build image
COPY . .

#install doxygen

# run doxygen to generate html document
RUN doxygen document/doxyfile

#use nginx as final image
FROM nginx AS final
WORKDIR /usr/share/nginx/html

RUN rm /usr/share/nginx/html/index.html
COPY --from=build /app/html /usr/share/nginx/html/document