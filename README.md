# Filter CSV by age

# Purpose
This program reads a CSV file that contains names and ages, filters out rows based on maximum age
that you choose, and then outputs the results either to the screen or to another CSV file.
It can read data from a file that you provide or directly from stdin which means you can also pipe text into it. 

When you run the program, the first argument you pass is always the maximum age that should be kept.
Any row with an age greater than that number will be skipped. The second argument (optional) is the input file name
and the third argument (also optional) is the output file name. If no input file is given, the program will read from stdin. 
If no output file is given, the filtered results will be printed to the terminal (stdout). 

The program also checks for errors and malformed lines. It skips lines that are empty, are formatted wrong (such as no comma, missing or invalid age).
It ignores comment lines tha start either with # or // and it trims space to make the data cleaner. 

# Example run 

Example command when reading from a file: 
```./csv.exe 18 people-with-age.csv out.csv```
This reads people-with-age.csv, filters all entries so only those aged 18 or younger remain,
and writes the result into out.csv. 

Example command when using stdin: 
```"Anna, 12" | ./csv.exe 18```
This takes the line "Anna 12" as input and prints it back because her age is below 18. 

Example when exporting stdin data into a file: 
```"Anna, 12" | ./csv.exe 18 > out.csv```
