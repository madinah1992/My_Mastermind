#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_CODE_LENGTH 5

void output_instructions();
void get_input(int argc, char *argv[], char **secret_code, int *max_attempts);
void generate_random_code(char **secret_code);
void take_guess(char *guess);
void evaluate_guess(char *guess, char *secret_code, int *well_placed, int *misplaced);
void play_game(char *secret_code, int max_attempts);

char* string_replica(const char *source);

int main(int argc, char *argv[]) {
    char *secret_code = NULL;
    int max_attempts = 10;

    get_input(argc, argv, &secret_code, &max_attempts);
    srand(time(NULL));
    if (secret_code == NULL) {
        generate_random_code(&secret_code);
    }
    output_instructions();
    play_game(secret_code, max_attempts);
    free(secret_code);
    return 0;
}

void output_instructions() {
    write(STDOUT_FILENO, "Will you find the secret code?\n", 31);
    write(STDOUT_FILENO, "Please enter a valid guess\n", 28);
}

void get_input(int argc, char *argv[], char **secret_code, int *max_attempts) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0) {
            if (i + 1 < argc) {
                *secret_code = string_replica(argv[i + 1]);
                i++;
            } else {
                write(STDERR_FILENO, "Error: No code provided after -c flag.\n", 39);
                exit(1);
            }
        } else if (strcmp(argv[i], "-t") == 0) {
            if (i + 1 < argc) {
                *max_attempts = atoi(argv[i + 1]);
                i++;
            } else {
                write(STDERR_FILENO, "Error: No value provided after -t flag.\n", 40);
                exit(1);
            }
        }
    }
}

void generate_random_code(char **secret_code) {
    char random_guess[MAX_CODE_LENGTH];
    for (int i = 0; i < 4; i++) {
        random_guess[i] = '0' + rand() % 10;
    }
    random_guess[4] = '\0';
    *secret_code = string_replica(random_guess);
}

char* string_replica(const char *source) {
    size_t length = strlen(source) + 1;
    char *duplicate = malloc(length);
    if (duplicate != NULL) {
        memcpy(duplicate, source, length);
    }
    return duplicate;
}

void take_guess(char *guess) {
    write(STDOUT_FILENO, ">", 1);
    char g;
    int index = 0;
    int valid_guess = 1;

    while (1) {
        if (read(STDIN_FILENO, &g, 1) <= 0 || g == '\n' || g == EOF) {
            if (index == 0) {
                write(STDOUT_FILENO, "\nGoodbye!\n", 10);
                exit(0);
            }
            break;
        }
        if (index < 4) {
            if (g < '0' || g > '9') {
                valid_guess = 0;
            }
            guess[index++] = g;
        }
    }
    guess[index] = '\0';

    if (index != 4 || !valid_guess) {
        write(STDOUT_FILENO, "\nWrong inputs, please enter a 4-digit number.\n", 47);
        take_guess(guess); // Prompt the user to guess again
    }
}

void evaluate_guess(char *guess, char *secret_code, int *well_placed, int *misplaced) {
    char temp_secret[MAX_CODE_LENGTH], temp_guess[MAX_CODE_LENGTH];
    strcpy(temp_secret, secret_code);
    strcpy(temp_guess, guess);

    *well_placed = 0;
    *misplaced = 0;

    // First pass: count well-placed pieces
    for (int i = 0; i < 4; i++) {
        if (temp_guess[i] == temp_secret[i]) {
            (*well_placed)++;
            temp_secret[i] = 'X'; // Mark as checked
            temp_guess[i] = 'Y';  // Mark as checked
        }
    }

    // Second pass: count misplaced pieces
    for (int i = 0; i < 4; i++) {
        if (temp_guess[i] != 'Y') {
            for (int j = 0; j < 4; j++) {
                if (temp_guess[i] == temp_secret[j] && temp_secret[j] != 'X') {
                    (*misplaced)++;
                    temp_secret[j] = 'X'; // Mark as checked
                    break;
                }
            }
        }
    }
}

void play_game(char *secret_code, int max_attempts) {
    int attempts = 0;
    while (attempts < max_attempts) {
        char guess[MAX_CODE_LENGTH];
        write(STDOUT_FILENO, "---\nRound ", 9);
        char attempt_str[3];
        snprintf(attempt_str, 3, "%d", attempts + 1);
        write(STDOUT_FILENO, attempt_str, strlen(attempt_str));
        write(STDOUT_FILENO, "\n", 1);
        take_guess(guess);

        if (strcmp(guess, secret_code) == 0) {
            write(STDOUT_FILENO, "Congratz! You did it!\n", 22);
            return;
        }

        int well_placed, misplaced;
        evaluate_guess(guess, secret_code, &well_placed, &misplaced);

        char well_placed_str[12];
        char misplaced_str[12];
        snprintf(well_placed_str, 12, "%d", well_placed);
        snprintf(misplaced_str, 12, "%d", misplaced);

        write(STDOUT_FILENO, "Well placed pieces: ", 20);
        write(STDOUT_FILENO, well_placed_str, strlen(well_placed_str));
        write(STDOUT_FILENO, "\n", 1);

        write(STDOUT_FILENO, "Misplaced pieces: ", 18);
        write(STDOUT_FILENO, misplaced_str, strlen(misplaced_str));
        write(STDOUT_FILENO, "\n", 1);

        attempts++;
    }

    write(STDOUT_FILENO, "Sorry, you've run out of attempts. The secret code was: ", 55);
    write(STDOUT_FILENO, secret_code, strlen(secret_code));
    write(STDOUT_FILENO, "\n", 1);
}
