main(ac, av)
char **av;
{
	char *bp, saltc[2], c;
	int salt, i;
	char *crypt(), *getpass();

	if (ac > 2) {
		printf("Usage: %s [password]\n", *av);
		exit(1);
	}
	if (ac == 2)
		bp = *++av;
	else
		bp = getpass("Password:");

/*
	time(&salt);
	salt += getpid();
	saltc[0] = salt & 077;
	saltc[1] = (salt>>6) & 077;
	for(i=0;i<2;i++){
		c = saltc[i] + '.';
		if(c>'9') c += 7;
		if(c>'Z') c += 6;
		saltc[i] = c;
	}
	printf("%s encrypts to %s\n", bp, crypt(bp, saltc));
*/
	printf("%s encrypts to %s\n", bp, crypt(bp, "mT"));
}
