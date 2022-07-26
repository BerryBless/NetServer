int _strlen(const char *str) {
	int len = 0;
	while (*(str + len) != '\0') { len++; }
	return len;
}

char *_strcpy(char *dest, const char *src) {
	int len = 0;
	while (*(src + len) != '\0') {
		*(dest + len) = *(src + len);
		len++;
	}
	*(dest + len) = '\0';
	return dest;
}

int _strcmp(const char *str1, const char *str2) {
	int len = 0;
	while (*(str1 + len) != '\0') {
		if (*(str1 + len) != *(str2 + len))
			return *(str1 + len) - *(str2 + len);
		len++;
	}
	return *(str1 + len) - *(str2 + len);
}

void _strcat(char *dest, const char *src) {
	int dLen = 0;
	int sLen = 0;
	while (*(dest + dLen) != '\0') { dLen++; }
	while (*(src + sLen) != '\0') {
		*(dest + dLen) = *(src + sLen);
		dLen++;
		sLen++;
	}
	*(dest + dLen) = '\0';
}

char *_strchr(const char *str, const char ch) {
	int len = 0;
	while (*(str + len) != '\0') {
		if (*(str + len) == ch)
			return  (char *)(str + len);
		len++;
	}
	return NULL;
}

char *_strstr(const char *str1, const char *str2) {
	int len1 = 0;
	int len2;
	while (*(str1 + len1) != '\0') {
		len2 = 0;
		while (*(str2 + len2) != '\0') {
			if (*(str1 + len1 + len2) != *(str2 + len2))
				break;
			len2++;
		}
		if (*(str2 + len2) == '\0') return (char *) (str1 + len1);
		len1++;
	}
	return NULL;
}

void __strlwr(char *str) {
	int Atoa = 'a' - 'A';
	int len = 0;
	while (*(str + len) != '\0') {
		if ('A' <= *(str + len) &&
			*(str + len) <= 'Z')
			*(str + len) += Atoa;
		len++;
	}
}