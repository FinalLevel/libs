#!/bin/bash

echo Building ${NAME} RPMs..
GITROOT=$(git rev-parse --show-toplevel)
cd $GITROOT

REPO=$(git config --get remote.origin.url)

REL=$(git rev-parse --short HEAD)git
RPMTOPDIR=$GITROOT/rpm-build
echo "Ver: $VER, Release: $REL, Repo: $REPO"

rm -rf $RPMTOPDIR

# Create tarball
mkdir -p $RPMTOPDIR/{SOURCES,SPECS}


git clone --recursive --depth 1 ${REPO} $RPMTOPDIR/SOURCES/${NAME}-${VER}-${REL}
cd $RPMTOPDIR/SOURCES/${NAME}-${VER}-${REL}/
sh autogen.sh
cd ..
tar -czf ${NAME}-${VER}-${REL}.tar.gz ${NAME}-${VER}-${REL}
cd $GITROOT

# Convert git log to RPM's ChangeLog format (shown with rpm -qp --changelog <rpm file>)
sed -e "s/%{ver}/$VER/" -e "s/%{rel}/$REL/" packaging/${NAME}.spec > $RPMTOPDIR/SPECS/${NAME}.spec
#git log --format="* %cd %aN%n- (%h) %s%d%n" --date=local | sed -r 's/[0-9]+:[0-9]+:[0-9]+ //' >> $RPMTOPDIR/SPECS/${NAME}.spec

# make RPMs directory
mkdir -p RPMS

# Build SRC and binary RPMs
rpmbuild		--define "_topdir $RPMTOPDIR" \
            --define "_rpmdir $PWD/RPMS"       \
            --define "_srcrpmdir $PWD/RPMS"    \
            --define '_rpmfilename %%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm' \
            -ba $RPMTOPDIR/SPECS/${NAME}.spec &&

rm -rf $RPMTOPDIR &&
echo Done
