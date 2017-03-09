#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifndef PACKAGE
#define PACKAGE "bmi"
#endif

#ifndef VERSION
#define VERSION "(git)"
#endif

static const char * const usage =
PACKAGE " " VERSION "\n\n"
"Perform BMI calculations from the command line. Usage:\n\n"
"  " PACKAGE " -v|--version            print version number\n"
"  " PACKAGE " -h|--help               show this message\n"
"  " PACKAGE " [unit]                  print a BMI table\n"
"  " PACKAGE " <height> [unit]         print a BMI table with weights\n"
"  " PACKAGE " <height> <bmi> [unit]   calculate weight (in unit) for BMI\n"
"  " PACKAGE " <height> <weight>       calculate BMI\n\n"
"Supported formats:\n\n"
"  height: 180cm 1.8m 6ft 6ft0 6\' 6\'0 \n"
"  weight: 80kg 175lb 175lbs 12.5st\n\n"
"Examples:\n\n"
"  " PACKAGE " 180cm 80kg     24.7 normal\n"
"  " PACKAGE " 1.8m 23        75kg\n"
"  " PACKAGE " 180cm 23 st    11.8st\n"
"  " PACKAGE " 23 st 180cm    11.8st\n"
"  " PACKAGE " 180cm          (a table)\n";

#define M_PER_CM 0.01
#define M_PER_FT 0.3048
#define M_PER_IN 0.0254 

#define KG_PER_LB 0.454
#define KG_PER_ST 6.35

#define BMI_VSUW_MAX 15
#define BMI_SUW_MAX  16
#define BMI_UW_MAX   18.5
#define BMI_OW_MIN   25
#define BMI_OB1_MIN  30
#define BMI_OB2_MIN  35
#define BMI_OB3_MIN  40

static void check_unexpected_suffix(const char *str, const char *arg)
{
	if (str[0]) {
		fprintf(stderr, PACKAGE ": unexpected suffix in: %s", arg);
		exit(EXIT_FAILURE);
	}
}

static void check_unset(double val, const char *valname, const char *arg)
{
	if (val) {
		fprintf(stderr, PACKAGE ": unexpected %s, already set: "
			"%s\n", valname, arg);
		exit(EXIT_FAILURE);
	}
}

static void check_weight_unset(double val, const char *unit, const char *arg)
{
	if (val) {
		check_unset(val, "weight", arg);
	} else if (unit) {
		fprintf(stderr, PACKAGE ": unexpected weight, unit already "
			"set: %s\n", arg);
		exit(EXIT_FAILURE);
	}
}

static const char *weight_str(double val, int precision, const char *unit,
	bool default_metric)
{
	static char buf[128];

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4996)  /* unsafe sprintf */
#endif

	if (((!unit || !unit[0]) && default_metric) || !strcmp("kg", unit)) {
		sprintf(buf, "%*.1f kg", precision, val);
	} else if ((!unit || !unit[0]) || !strcmp("lb", unit)) {
		sprintf(buf, "%*.1f lb", precision, val / KG_PER_LB);
	} else if (!strcmp("lbs", unit)) {
		sprintf(buf, "%*.1f lbs", precision, val / KG_PER_LB);
	} else if (!strcmp("st", unit)) {
		sprintf(buf, "%*.2f st", precision, val / KG_PER_ST);
	} else {
		fprintf(stderr, PACKAGE ": unknown weight unit: %s\n",
			unit);
		exit(EXIT_FAILURE);
	}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

	return buf;
}

int main(int argc, char **argv)
{
	double height_m = 0, weight_kg = 0, bmi = 0;
	const char *weight_unit = NULL;
	bool height_metric = true;

	for (int i = 1; i < argc; i++) {
		const char *arg = argv[i];
		if (!strcmp(arg, "-v") || !strcmp(arg, "--version")) {
			printf(PACKAGE " " VERSION "\n");
			exit(EXIT_SUCCESS);
		} else if (!strcmp(arg, "-h") || !strcmp(arg, "--help")) {
			printf("%s", usage);
			exit(EXIT_SUCCESS);
		} else if (arg[0] == '-') {
			fprintf(stderr, PACKAGE ": unknown argument: %s\n",
				arg);
			exit(EXIT_FAILURE);
		}

		char *rest;
		double val = strtod(argv[i], &rest);

		if (!strcmp("kg", rest)) {
			check_weight_unset(weight_kg, weight_unit, arg);
			weight_kg = val;
			weight_unit = rest;
		} else if (!strcmp("lb", rest) || !strcmp("lbs", rest)) {
			check_weight_unset(weight_kg, weight_unit, arg);
			weight_kg = val * KG_PER_LB;
			weight_unit = rest;
		} else if (!strcmp("st", rest)) {
			check_weight_unset(weight_kg, weight_unit, arg);
			weight_kg = val * KG_PER_ST;
			weight_unit = rest;
		} else if (!val) {
			fprintf(stderr, PACKAGE ": expecting a value: %s\n",
				arg);
			exit(EXIT_FAILURE);
		} else if (!rest[0]) {
			check_unset(bmi, "BMI", arg);
			bmi = val;
		} else if (!strcmp("cm", rest)) {
			check_unset(height_m, "height", arg);
			height_m = val * M_PER_CM;
		} else if (!strcmp("m", rest)) {
			check_unset(height_m, "height", arg);
			height_m = val;
		} else if (rest[0] == '\'') {
			check_unset(height_m, "height", arg);
			height_m = val * M_PER_FT;
			height_m += strtod(&rest[1], &rest) * M_PER_IN;
			height_metric = false;
			check_unexpected_suffix(rest, arg);
		} else if (strncmp("ft", rest, 2)) {
			check_unset(height_m, "height", arg);
			height_m = val * M_PER_FT;
			height_m += strtod(&rest[2], &rest) * M_PER_IN;
			height_metric = false;
			check_unexpected_suffix(rest, arg);
		} else {
			fprintf(stderr, PACKAGE ": unknown unit type: %s\n",
				arg);
			exit(EXIT_FAILURE);
		}
	}

	if (weight_kg && bmi) {
		fprintf(stderr, PACKAGE ": both weight and BMI given - "
			"omit one or both.\n");
		exit(EXIT_FAILURE);
	} else if (height_m && weight_kg) {
		bmi = weight_kg / (height_m * height_m);
		if (bmi < BMI_VSUW_MAX) {
			printf("%.1f (very severely underweight)\n", bmi);
		} else if (bmi < BMI_SUW_MAX) {
			printf("%.1f (severely underweight)\n", bmi);
		} else if (bmi < BMI_UW_MAX) {
			printf("%.1f (underweight)\n", bmi);
		} else if (bmi < BMI_OW_MIN) {
			printf("%.1f (normal)\n", bmi);
		} else if (bmi < BMI_OB1_MIN) {
			printf("%1.f (overweight)\n", bmi);
		} else if (bmi < BMI_OB2_MIN) {
			printf("%1.f (obese I)\n", bmi);
		} else if (bmi < BMI_OB3_MIN) {
			printf("%1.f (obese II)\n", bmi);
		} else {
			printf("%1.f (obese III)\n", bmi);
		}
	} else if (height_m && bmi) { 
		weight_kg = bmi * height_m * height_m;
		printf("%s\n", weight_str(weight_kg, 1, weight_unit,
			height_metric));
	} else if (height_m) {
		printf(" BMI   class                         weight\n"
			"-------------------------------------------\n");
		printf(" 15    very severely underweight  %s\n",
			weight_str(BMI_VSUW_MAX * height_m * height_m,
				6, weight_unit, height_metric));
		printf(" 16    severely underweight       %s\n",
			weight_str(BMI_SUW_MAX * height_m * height_m,
				6, weight_unit, height_metric));
		printf(" 18.5  underweight                %s\n",
			weight_str(BMI_UW_MAX * height_m * height_m,
				6, weight_unit, height_metric));
		printf("       (normal)\n"
			" 25    overweight                 %s\n",
			weight_str(BMI_OW_MIN * height_m * height_m,
				6, weight_unit, height_metric));
		printf(" 30    obese I                    %s\n",
			weight_str(BMI_OB1_MIN * height_m * height_m,
				6, weight_unit, height_metric));
		printf(" 35    obese II                   %s\n",
			weight_str(BMI_OB2_MIN * height_m * height_m,
				6, weight_unit, height_metric));
		printf(" 40    obese III                  %s\n",
			weight_str(BMI_OB3_MIN * height_m * height_m,
				6, weight_unit, height_metric));
	} else if (weight_kg || bmi) {
		fprintf(stderr, PACKAGE ": no height given.\n");
		exit(EXIT_FAILURE);
	} else {
		printf(
			"Run  " PACKAGE " -h  for usage information.\n\n"
			" BMI   class\n"
			"--------------------------------\n"
			" 15    very severely underweight\n"
			" 16    severely underweight\n"
			" 18.5  underweight\n"
			"       (normal)\n"
			" 25    overweight\n"
			" 30    obese I\n"
			" 35    obese II\n");
	}

	return 0;
}
