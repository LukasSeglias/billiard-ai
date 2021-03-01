FROM danteev/texlive:202010010206558488ac

RUN apt-get update && apt-get install -y dos2unix

ADD compile.sh /usr/local/bin/compile.sh
RUN chmod 777 /usr/local/bin/compile.sh
RUN dos2unix /usr/local/bin/compile.sh

ENTRYPOINT ["/usr/local/bin/compile.sh"]
#ENTRYPOINT [""]