This directory is for developer helper scripts, like those used to automate
the creation of controller layouts.

## Dummy HID Device (Linux only)

If you ever need a dummy HID device to test mapping capabilities or introspect reports set by a mapping for example, you can build a dummy one using `dummy_hid_device.c`. Note that you will need `uhid` to use this daemon.

Here is how to get the daemon setup. Make sure to do this **before** Mixxx is started

```sh
# Enable UHID
sudo modprobe uhid
# This next command assumes you are running it from the Mixxx directory
#  Optionally, you can pass:
#    `-DVENDOR_ID=0x1234` to customize vendor ID
#    `-DPRODUCT_ID=0x1234` to customize product ID
#    `-DDEVICE_NAME="My Device"` to customize the device name
cd build && gcc ../tools/dummy_hid_device.c -lhidapi-hidraw -o dummy_hid_device && sudo ./dummy_hid_device
# Allow the created hidraw device to be accessed by the user. You may also set the write udev rules. Finally, you can also run Mixxx as root, but that's not recommended.
sudo chown "$USER" "$(ls -1t /dev/hidraw* | head -n 1)"
```

## Beat analyzer

In order to assess the quality of the beat analyzer, you can use the following command:

```
python3 tools/analyzer_benchmark.py /path/to/dataset.csv
```

You can use the following envvar:

- `DOWNLOAD_FOLDER`: Path the folder where to download music. This is only used if your dataset contains remote files. It will use the current directory if unset.
- `DB_PATH`: Path to your Mixxx DB. THis will be used to complete your dataset, with expected BPM and grid position.

### Dataset

Your dataset need to include:

- `source`: The URL to the file. Can be either an HTTP or local URL
- `title`: The track title. Can be an empty string if using a local file
- `artist`: The track artist. Can be an empty string if using a local file
- `tags`: A comma-separated list of tags to indicate genre or specificity of the track. This is useful to detect patterns.

Example:

```csv
source,title,artist,tags
/path/to/my/file,,Four to the floor electro
```
