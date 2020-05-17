void logprintf(char* template, int context) {
    FILE *fp;

    fp = fopen("./output.log", "a+");
    fprintf(fp, template, context);
    fclose(fp);
}