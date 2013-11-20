if [ ! -e "debian/usr/lib/mixxx/plugins/libsoundsourcem4a.so" ]; then
    printf "Error: libsoundsourcem4a.so not found.\n"
    printf "Please put the libsoundsourcem4a.so file you want to package into the debian/usr/lib/mixxx/plugins directory \n"
    exit
fi
dpkg-deb --build debian
mv debian.deb mixxx-m4a_1.8.0~beta1.deb
