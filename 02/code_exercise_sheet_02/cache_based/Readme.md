# Build

To build the library, use the following command:

```
make lib
```

After you have built the library, build the program as follows:

```
make
```

-----

# Usage

Measurements are better with power-saving options turned off.

To find out timings, use the following two commands:

```
./exercise_plot > measurement.csv
python3 exercise_plot.py
```

Call `exercise_receiver` with a threshold value you have determined.

```
./exercise_receiver xxx
```
