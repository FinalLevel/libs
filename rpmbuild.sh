#!/bin/bash

echo Building ${NAME} RPMs..

PROJECT_PATH="$( cd "$(dirname "$0")" ; pwd -P )"

if [  -z "$REPO" ]; then
	REPO=$(git config --get remote.origin.url)
fi

RPMTOPDIR=$PROJECT_PATH/rpm-build


rm -rf $RPMTOPDIR

# Create tarball
mkdir -p $RPMTOPDIR/{SOURCES,SPECS}


git clone --recursive --depth 1 ${REPO} $RPMTOPDIR/SOURCES/${NAME}-${VER}
cd $RPMTOPDIR/SOURCES/${NAME}-${VER}/
REL=$(git rev-parse --short HEAD)git
echo "Ver: $VER, Release: $REL, Repo: $REPO, Path: $RPMTOPDIR"
sed -e "s/%{ver}/$VER/" -e "s/%{rel}/$REL/" packaging/${NAME}.spec > $RPMTOPDIR/SPECS/${NAME}.spec
#git log --format="* %cd %aN%n- (%h) %s%d%n" --date=local | sed -r 's/[0-9]+:[0-9]+:[0-9]+ //' >> $RPMTOPDIR/SPECS/${NAME}.spec

sh autogen.sh

cd ..
mv ${NAME}-${VER} ${NAME}-${VER}-${REL}
tar -czf ${NAME}-${VER}-${REL}.tar.gz ${NAME}-${VER}-${REL}
cd $PROJECT_PATH

# make RPMs directory
mkdir -p RPMS

# Build SRC and binary RPMs
rpmbuild		--define "_topdir $RPMTOPDIR" \
            --define "_rpmdir $PROJECT_PATH/RPMS"       \
            --define "_srcrpmdir $PROJECT_PATH/RPMS"    \
            --define '_rpmfilename %%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm' \
            -ba $RPMTOPDIR/SPECS/${NAME}.spec &&

rm -rf $RPMTOPDIR &&
echo Done
