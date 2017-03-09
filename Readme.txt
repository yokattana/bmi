bmi
---

Perform BMI calculations from the command line. Usage:

  bmi -v|--version            print version number
  bmi -h|--help               show this message
  bmi [unit]                  print a BMI table
  bmi <height> [unit]         print a BMI table with weights
  bmi <height> <bmi> [unit]   calculate weight (in unit) for BMI
  bmi <height> <weight>       calculate BMI

Supported formats:

  height: 180cm 1.8m 6ft 6ft0 6' 6'0 
  weight: 80kg 175lb 175lbs 12.5st

Examples:

  bmi 180cm 80kg     24.7 normal
  bmi 1.8m 23        75kg
  bmi 180cm 23 st    11.8st
  bmi 23 st 180cm    11.8st
  bmi 180cm          (a table)
