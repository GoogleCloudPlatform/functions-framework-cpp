# Functions Framework for C++: Test Install Targets

This program is used in the CI builds to verify `make install` produces usable
artifacts.  It is not enough that `make install` runs and exits without error,
we also want to ensure the installed artifacts are usable by some program.
