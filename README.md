# Unikraft "hello-world" Application with Coverage Support (gcov)

This builds the Unikraft hello-world image with coverage support used by [gcov](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html).
The actual support is provided in gcc via the `-fprofile-arcs` option.
Coverage support can be used for profile-guided optimization (PGO).
This is why the usual way to compile and link a program under test is via the `-fprofile-generate` option; it includes the `-fprofile-arcs` option.

## Dependencies

The following libraries / dependencies are required:
* `lib-pthread-embedded`
* `lib-libunwind`
* `lib-compiler-rt`
* `lib-libcxxabi`
* `lib-libcxx`
* `lib-newlib`
* `lib-gcc`

For `lib-gcc` use the [UPB clone](https://github.com/cs-pub-ro/lib-gcc), the `gcov` branch.
Similarly, for Unikraft use [UPB clone](https://github.com/cs-pub-ro/unikraft), the `gcov` branch.

For the other libraries, use the `staging` branch.

The dependencies have to be included in that order as done in the `Makefile`.
Update the `Makefile` variables (`UK_ROOT`, `UK_LIBS`, `LIBS`) according to your setup.

## Configuration

Enter the configuration screen:
```
make menuconfig
```
Enable the following configuration options:
* `Platform Configuration` -> `KVM (guest)`
* `Library Configuration`
  * `libpthread-embedded`
  * `libunwind`
  * `compiler-rt`
  * `libcxxabi`
  * `libcxx`
  * `newlib`
  * `gcc`
    * `libgcov: A Profiling Library`
  * `vfscore: Configuration`
    * `Automatically mount a root filesystem (/)`
    * `Default root filesystem`: Choose `9pfs`
    * `Default root device`: Choose `fs0`
  * `POSIX sysinfo: Information about system parameters`
* `Build Options`
  * `PGO - Profile-guided optimization`

You can validate the configuration options, by diffing the `.config` file to the `config.sample` file:
```
diff .config config.sample
```

An application with coverage support can be built in two ways:
1. to generate the profiling information: the `-fprofile-generate` option
1. to use the generated information for optimization, i.e. the actual profile-guided optimization (PGO): the `-fprofile-use` option

By default, the application is built to generate the profile, i.e. using the `-fprofile-generate` option.

If building the application for optimization, i.e. using the `-fprofile-use` option, two configuration steps are required:
* In the configuration screen (`make menuconfig`), select `Build options` -> `PGO - Profile-guided optimization` -> `PGO Options` -> `profile-use`.
* In the `Makefile.uk` file, comment out the line with the `-fprofile-generate` option and uncomment the line with the `-fprofile-use` option.
  In the end the lines should be:
```
#APPHELLOWORLD_CFLAGS-$(CONFIG_OPTIMIZE_PGO_GENERATE) += -fprofile-generate
APPHELLOWORLD_CFLAGS-$(CONFIG_OPTIMIZE_PGO_USE) += -fprofile-use
```

## Build

Build the application by running
```
make
```
The first time you do it will take some time, as the library files are downloaded, unpacked and built.
The resulting KVM image is `build/app-hello-world-gcov_kvm-x86_64`.

## Run

The profiling information is generated as a `.gcda` file.
Create a folder named `fs0` (the name is not important):
```
mkdir fs0
```
The `fs0` will be mapped as the root directory in the Unikernel via the 9pfs filesystem support.

Use the [`qemu-guest` script in kraft](https://github.com/unikraft/kraft/blob/staging/scripts/qemu-guest) to run the Unikraft application in a KVM virtual machine:
```
qemu-guest -k build/app-hello-world-gcov_kvm-x86_64 -e fs0
```
The script requires `root` privileges to run.

After running the profile generation image, the output file will be generated in `fs0/gcov_profiling/main.gcda`.
The file will be use by the profile use phase after re-building the application for optimization.
The file is created with `root` privileges by the `qemu-script`.
Modifying or removing it will require `root` privileges.
