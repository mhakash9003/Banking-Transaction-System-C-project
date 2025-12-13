#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Define the structure for an account record
struct account {
    int account_no;
    char name[50];
    double balance;
};

// Global file name for data persistence
const char* FILENAME = "account_records.txt";

// --- Function Prototypes ---
void main_menu();
void create_account();
void view_account_details();
void deposit_amount();
void withdraw_amount();
void transfer_funds();
void delete_account();
void view_all_accounts();
int search_and_update(int account_no, double amount, int type); // type: 1=Deposit, 2=Withdraw, 3=Transfer-Out, 4=Transfer-In, 5=Delete
int get_account_details(int account_no, struct account *acc_out);

// --- Main Function ---
int main() {
    main_menu();
    return 0;
}

// --- Menu System ---
void main_menu() {
    int choice;
    do {
        system("clear || cls"); // Clear screen (for Unix/Windows)
        printf("==================================================\n");
        printf("  BANKING TRANSACTION MANAGEMENT SYSTEM (BTMS)\n");
        printf("==================================================\n");
        printf("1. Create New Account\n");
        printf("2. View Account Details\n");
        printf("3. Deposit Amount\n");
        printf("4. Withdraw Amount\n");
        printf("5. Transfer Funds\n");
        printf("6. Delete Account\n");
        printf("7. View All Accounts\n");
        printf("8. Exit\n");
        printf("--------------------------------------------------\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: create_account(); break;
            case 2: view_account_details(); break;
            case 3: deposit_amount(); break;
            case 4: withdraw_amount(); break;
            case 5: transfer_funds(); break;
            case 6: delete_account(); break;
            case 7: view_all_accounts(); break;
            case 8: printf("\nThank you for using the BTMS. Exiting...\n"); break;
            default: printf("\nInvalid choice. Please try again.\n");
        }
        if (choice != 8) {
            printf("\nPress Enter to continue...");
            while (getchar() != '\n'); // Clear buffer
            getchar(); // Wait for user input
        }
    } while (choice != 8);
}

// --- CORE FUNCTIONALITIES ---

/**
 * @brief Creates a new account and writes it to the file.
 */
void create_account() {
    struct account new_acc;
    FILE *fp;

    printf("\n--- CREATE NEW ACCOUNT ---\n");
    printf("Enter Account Number (e.g., 1001): ");
    scanf("%d", &new_acc.account_no);
    printf("Enter Name: ");
    while (getchar() != '\n'); // Clear buffer
    fgets(new_acc.name, 50, stdin);
    new_acc.name[strcspn(new_acc.name, "\n")] = 0; // Remove trailing newline

    // Simple validation to check if account already exists
    if (get_account_details(new_acc.account_no, &new_acc) == 1) {
        printf("\nERROR: Account Number %d already exists.\n", new_acc.account_no);
        return;
    }

    printf("Enter Initial Deposit Amount: ");
    if (scanf("%lf", &new_acc.balance) != 1 || new_acc.balance < 0) {
        printf("\nERROR: Invalid deposit amount.\n");
        return;
    }

    fp = fopen(FILENAME, "ab"); // Open file in append binary mode
    if (fp == NULL) {
        printf("\nERROR: Could not open file for writing.\n");
        return;
    }

    // Write the new account structure to the file
    fwrite(&new_acc, sizeof(struct account), 1, fp);
    fclose(fp);
    printf("\nAccount %d Created Successfully!\n", new_acc.account_no);
}

/**
 * @brief Views the details of a specific account by account number.
 */
void view_account_details() {
    int acc_no;
    struct account acc;

    printf("\n--- VIEW ACCOUNT DETAILS ---\n");
    printf("Enter Account Number: ");
    scanf("%d", &acc_no);

    if (get_account_details(acc_no, &acc) == 1) {
        printf("\nAccount Found:\n");
        printf("--------------------------------\n");
        printf("Account No: %d\n", acc.account_no);
        printf("Name      : %s\n", acc.name);
        printf("Balance   : %.2lf\n", acc.balance);
        printf("--------------------------------\n");
    } else {
        printf("\nERROR: Account Number %d not found.\n", acc_no);
    }
}

/**
 * @brief Deposits a specified amount into an account.
 */
void deposit_amount() {
    int acc_no;
    double amount;

    printf("\n--- DEPOSIT FUNDS ---\n");
    printf("Enter Account Number: ");
    scanf("%d", &acc_no);
    printf("Enter Amount to Deposit: ");
    if (scanf("%lf", &amount) != 1 || amount <= 0) {
        printf("\nERROR: Invalid deposit amount.\n");
        return;
    }

    if (search_and_update(acc_no, amount, 1)) {
        printf("\nDeposit Successful! Account %d updated.\n", acc_no);
    } else {
        printf("\nERROR: Deposit failed. Account not found.\n");
    }
}

/**
 * @brief Withdraws a specified amount from an account. Includes overdraft check.
 */
void withdraw_amount() {
    int acc_no;
    double amount;

    printf("\n--- WITHDRAW FUNDS ---\n");
    printf("Enter Account Number: ");
    scanf("%d", &acc_no);
    printf("Enter Amount to Withdraw: ");
    if (scanf("%lf", &amount) != 1 || amount <= 0) {
        printf("\nERROR: Invalid withdrawal amount.\n");
        return;
    }

    if (search_and_update(acc_no, amount, 2)) {
        printf("\nWithdrawal Successful! Account %d updated.\n", acc_no);
    } else {
        // search_and_update returns 0 for not found or insufficient balance
        struct account acc;
        if (get_account_details(acc_no, &acc) == 1 && acc.balance < amount) {
            printf("\nERROR: Insufficient Balance. Current Balance: %.2lf\n", acc.balance);
        } else {
            printf("\nERROR: Withdrawal failed. Account not found.\n");
        }
    }
}

/**
 * @brief Transfers funds between two accounts.
 */
void transfer_funds() {
    int source_acc_no, dest_acc_no;
    double amount;
    struct account source_acc;

    printf("\n--- FUND TRANSFER ---\n");
    printf("Enter Source Account Number: ");
    scanf("%d", &source_acc_no);
    printf("Enter Destination Account Number: ");
    scanf("%d", &dest_acc_no);

    if (source_acc_no == dest_acc_no) {
        printf("\nERROR: Cannot transfer to the same account.\n");
        return;
    }

    printf("Enter Amount to Transfer: ");
    if (scanf("%lf", &amount) != 1 || amount <= 0) {
        printf("\nERROR: Invalid transfer amount.\n");
        return;
    }

    // 1. Check if source account exists and has sufficient funds
    if (get_account_details(source_acc_no, &source_acc) == 0) {
        printf("\nERROR: Source Account %d not found.\n", source_acc_no);
        return;
    }
    if (source_acc.balance < amount) {
        printf("\nERROR: Insufficient Balance in Source Account. Current Balance: %.2lf\n", source_acc.balance);
        return;
    }

    // 2. Check if destination account exists
    struct account dest_acc;
    if (get_account_details(dest_acc_no, &dest_acc) == 0) {
        printf("\nERROR: Destination Account %d not found.\n", dest_acc_no);
        return;
    }

    // 3. Process the transfer (Withdraw from source, Deposit to destination)
    // Withdraw from source
    if (search_and_update(source_acc_no, amount, 3) == 0) { // type 3 for Transfer-Out
        printf("\nERROR: Failed to withdraw from Source Account.\n");
        return;
    }

    // Deposit to destination
    if (search_and_update(dest_acc_no, amount, 4) == 0) { // type 4 for Transfer-In
        printf("\nERROR: Failed to deposit to Destination Account. Transaction aborted (Source account was debited).\n");
        // In a real system, a reversal/compensation transaction would be logged here.
        return;
    }

    printf("\nFund Transfer of %.2lf from %d to %d Successful!\n", amount, source_acc_no, dest_acc_no);
}

/**
 * @brief Deletes an account record.
 */
void delete_account() {
    int acc_no;
    printf("\n--- DELETE ACCOUNT ---\n");
    printf("Enter Account Number to Delete: ");
    scanf("%d", &acc_no);

    if (search_and_update(acc_no, 0, 5)) { // type 5 for Delete
        printf("\nAccount %d deleted successfully.\n", acc_no);
    } else {
        printf("\nERROR: Account %d not found or deletion failed.\n", acc_no);
    }
}

/**
 * @brief Reads and displays all account records in the file.
 */
void view_all_accounts() {
    FILE *fp;
    struct account acc;
    int count = 0;

    fp = fopen(FILENAME, "rb"); // Open file in read binary mode
    if (fp == NULL) {
        printf("\nNo accounts found. File does not exist or is empty.\n");
        return;
    }

    printf("\n==================================================\n");
    printf("           ALL CUSTOMER ACCOUNT RECORDS\n");
    printf("==================================================\n");
    printf("ACC NO | %-30s | BALANCE\n", "NAME");
    printf("--------------------------------------------------\n");

    // Read records one by one until EOF
    while (fread(&acc, sizeof(struct account), 1, fp) == 1) {
        printf("%6d | %-30s | %.2lf\n", acc.account_no, acc.name, acc.balance);
        count++;
    }

    printf("--------------------------------------------------\n");
    printf("Total Accounts: %d\n", count);
    fclose(fp);
}

// --- UTILITY FUNCTIONS ---

/**
 * @brief Searches for an account and updates its balance or deletes it.
 * @param account_no The account number to search for.
 * @param amount The amount to use for the operation.
 * @param type 1=Deposit, 2=Withdraw, 3=Transfer-Out, 4=Transfer-In, 5=Delete.
 * @return 1 on success, 0 on failure (not found or insufficient balance).
 */
int search_and_update(int account_no, double amount, int type) {
    FILE *fp_in, *fp_out;
    struct account acc;
    int found = 0;

    fp_in = fopen(FILENAME, "rb");
    if (fp_in == NULL) return 0;

    // Open a temporary file for writing updated records
    fp_out = fopen("temp_records.txt", "wb");
    if (fp_out == NULL) {
        fclose(fp_in);
        return 0;
    }

    while (fread(&acc, sizeof(struct account), 1, fp_in) == 1) {
        if (acc.account_no == account_no) {
            found = 1;
            if (type == 1 || type == 4) { // Deposit or Transfer-In
                acc.balance += amount;
                fwrite(&acc, sizeof(struct account), 1, fp_out);
            } else if (type == 2 || type == 3) { // Withdraw or Transfer-Out
                if (acc.balance >= amount) {
                    acc.balance -= amount;
                    fwrite(&acc, sizeof(struct account), 1, fp_out);
                } else {
                    // Overdraft check for withdraw/transfer-out failed,
                    // write the old record back and don't mark as successful (found=0 below)
                    fwrite(&acc, sizeof(struct account), 1, fp_out);
                    found = 0; // Operation failed due to insufficient funds
                    break;
                }
            } else if (type == 5) { // Delete - do nothing (don't write to fp_out)
                // Successfully found and deleted by skipping it
            }
        } else {
            // Write non-matching records to the temp file
            fwrite(&acc, sizeof(struct account), 1, fp_out);
        }
    }

    fclose(fp_in);
    fclose(fp_out);

    // Replace the old file with the temporary file if update was successful
    if (found == 1 || type == 5) {
        remove(FILENAME);
        rename("temp_records.txt", FILENAME);
        return 1;
    } else {
        remove("temp_records.txt"); // Clean up temp file
        return 0; // Account not found or insufficient balance/operation failed
    }
}

/**
 * @brief Searches for an account and loads its details into acc_out.
 * @param account_no The account number to search for.
 * @param acc_out Pointer to the struct where data will be loaded.
 * @return 1 if found, 0 if not found.
 */
int get_account_details(int account_no, struct account *acc_out) {
    FILE *fp;
    struct account temp_acc;
    int found = 0;

    fp = fopen(FILENAME, "rb");
    if (fp == NULL) return 0;

    // Linear search for the account number
    while (fread(&temp_acc, sizeof(struct account), 1, fp) == 1) {
        if (temp_acc.account_no == account_no) {
            *acc_out = temp_acc; // Copy the found structure
            found = 1;
            break;
        }
    }
    
    fclose(fp);
    return found;
}