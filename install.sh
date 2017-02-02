BBSUSERHOME=/home/bbs
mkdir -p ${BBSUSERHOME}/bin/
mkdir -p ${BBSUSERHOME}/core/bbs/
mkdir -p ${BBSUSERHOME}/core/bbsqueued/
mkdir -p ${BBSUSERHOME}/data/
mkdir -p ${BBSUSERHOME}/etc/who/
mkdir -p ${BBSUSERHOME}/help/
mkdir -p ${BBSUSERHOME}/message/desc/
mkdir -p ${BBSUSERHOME}/var/
cp -f bbs ${BBSUSERHOME}/bin
chmod 755 ${BBSUSERHOME}/bin/bbs
./setupbbs
sudo chown -R bbs:bbs ${BBSUSERHOME}
