#include <stdio.h>
#include <string.h>

#define STANDARD_TICKET_PRICE 150.0
#define PREMIUM_TICKET_PRICE 250.0
#define RECLINER_TICKET_PRICE 400.0

#define DISCOUNT_PERCENTAGE 10.0
#define PERCENTAGE_DIVISOR 100.0

enum TicketCategory{
    STANDARD_CATEGORY_CHOICE = 1,
    PREMIUM_CATEGORY_CHOICE,
    RECLINER_CATEGORY_CHOICE
};

enum BookingLimits{
    MINIMUM_TICKET_COUNT = 1,
    MAXIMUM_TICKET_COUNT = 20,
    DISCOUNT_ELIGIBILITY_COUNT = 5,
    CUSTOMER_NAME_LENGTH = 50,
    CATEGORY_NAME_LENGTH = 20
};

void clearInputBuffer(){
    int inputCharacter;

    while ((inputCharacter = getchar()) != '\n' && inputCharacter != EOF){}
}

int readInteger(const char prompt[], int *value){
    while (1){
        printf("%s", prompt);

        if (scanf("%d", value) == 1){
            clearInputBuffer();
            return 1;
        }

        printf("Error: Please enter a whole number.\n");
        clearInputBuffer();
    }
}

int readBookingDetails(char customerName[], int *ticketCount, int *categoryChoice){
    printf("Enter customer name: ");

    if (scanf(" %49[^\n]", customerName) != 1){
        printf("Error: Invalid customer name.\n");
        return 0;
    }

    clearInputBuffer();

    while (1){
        if (!readInteger("Enter number of tickets: ", ticketCount)){
            return 0;
        }

        if (*ticketCount >= MINIMUM_TICKET_COUNT && *ticketCount <= MAXIMUM_TICKET_COUNT){
            break;
        }

        printf("Error: Ticket count must be between %d and %d. Please try again.\n", MINIMUM_TICKET_COUNT, MAXIMUM_TICKET_COUNT);
    }

    while (1){
        if (!readInteger("Select ticket category (1 = Standard, 2 = Premium, 3 = Recliner): ", categoryChoice)){
            return 0;
        }

        if (*categoryChoice >= STANDARD_CATEGORY_CHOICE && *categoryChoice <= RECLINER_CATEGORY_CHOICE){
            break;
        }

        printf("Error: Invalid category. Please choose 1, 2, or 3.\n");
    }

    return 1;
}

int selectTicketCategory(int categoryChoice, char categoryName[], double *ticketPrice){
    switch (categoryChoice){

        case STANDARD_CATEGORY_CHOICE:
            strcpy(categoryName, "Standard");
            *ticketPrice = STANDARD_TICKET_PRICE;
            return 1;

        case PREMIUM_CATEGORY_CHOICE:
            strcpy(categoryName, "Premium");
            *ticketPrice = PREMIUM_TICKET_PRICE;
            return 1;

        case RECLINER_CATEGORY_CHOICE:
            strcpy(categoryName, "Recliner");
            *ticketPrice = RECLINER_TICKET_PRICE;
            return 1;

        default:
            return 0;
    }
}

void calculateBill(int ticketCount, double ticketPrice, double *subtotal, double *discountAmount, double *finalBill){

    *subtotal = ticketCount * ticketPrice;

    if (ticketCount >= DISCOUNT_ELIGIBILITY_COUNT){
        *discountAmount = *subtotal * DISCOUNT_PERCENTAGE / PERCENTAGE_DIVISOR;
    }
    else{
        *discountAmount = 0.0;
    }

    *finalBill = *subtotal - *discountAmount;
}

void displayBookingDetails(const char customerName[], const char categoryName[], int ticketCount, double subtotal, double discountAmount, double finalBill){
    printf("\n----- Booking Details -----\n");
    printf("Customer: %s\n", customerName);
    printf("Ticket Category: %s\n", categoryName);
    printf("Tickets: %d\n", ticketCount);
    printf("Subtotal: %.2f\n", subtotal);
    printf("Discount: %.2f\n", discountAmount);
    printf("Final Bill: %.2f\n", finalBill);
}

int main(){
    char customerName[CUSTOMER_NAME_LENGTH];
    char categoryName[CATEGORY_NAME_LENGTH];
    int ticketCount;
    int categoryChoice;
    double ticketPrice;
    double subtotal;
    double discountAmount;
    double finalBill;

    if (!readBookingDetails(customerName, &ticketCount, &categoryChoice)){
        return 1;
    }

    if (!selectTicketCategory(categoryChoice, categoryName, &ticketPrice)){
        printf("Error: Invalid ticket category.\n");
        return 1;
    }

    calculateBill(ticketCount, ticketPrice, &subtotal, &discountAmount, &finalBill);
    displayBookingDetails(customerName, categoryName, ticketCount, subtotal, discountAmount, finalBill);

    return 0;
}
