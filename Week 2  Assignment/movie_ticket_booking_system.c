#include <stdio.h>
#include <string.h>

#define STANDARD_TICKET_PRICE 150.0
#define PREMIUM_TICKET_PRICE 250.0
#define RECLINER_TICKET_PRICE 400.0
#define DISCOUNT_PERCENTAGE 10.0
#define PERCENTAGE_DIVISOR 100.0
#define MINIMUM_TICKET_COUNT 1
#define MAXIMUM_TICKET_COUNT 20
#define DISCOUNT_ELIGIBILITY_COUNT 5
#define CUSTOMER_NAME_LENGTH 50
#define CATEGORY_NAME_LENGTH 20

enum TicketCategory {
    STANDARD_CATEGORY_CHOICE = 1,
    PREMIUM_CATEGORY_CHOICE,
    RECLINER_CATEGORY_CHOICE
};

typedef struct {
    char customerName[CUSTOMER_NAME_LENGTH];
    char categoryName[CATEGORY_NAME_LENGTH];
    int ticketCount;
    int categoryChoice;
    double ticketPrice;
    double subtotal;
    double discountAmount;
    double finalBill;
} Booking;

void clearInputBuffer(void) {
    int inputCharacter;

    while ((inputCharacter = getchar()) != '\n' && inputCharacter != EOF) {
    }
}

int readInteger(const char prompt[], int *value) {
    while (1) {
        int inputStatus;

        printf("%s", prompt);
        inputStatus = scanf("%d", value);

        if (inputStatus == 1) {
            clearInputBuffer();
            return 1;
        }

        if (inputStatus == EOF) {
            printf("\nError: Input ended unexpectedly.\n");
            return 0;
        }

        printf("Error: Please enter a whole number.\n");
        clearInputBuffer();
    }
}

int readBookingDetails(Booking *booking) {
    printf("Enter customer name: ");

    if (scanf(" %49[^\n]", booking->customerName) != 1) {
        printf("Error: Invalid customer name.\n");
        return 0;
    }

    clearInputBuffer();

    while (1) {
        if (!readInteger("Enter number of tickets: ", &booking->ticketCount)) {
            return 0;
        }

        if (booking->ticketCount >= MINIMUM_TICKET_COUNT && booking->ticketCount <= MAXIMUM_TICKET_COUNT) {
            break;
        }

        printf("Error: Ticket count must be between %d and %d. Please try again.\n", MINIMUM_TICKET_COUNT, MAXIMUM_TICKET_COUNT);
    }

    while (1) {
        if (!readInteger("Select ticket category (1 = Standard, 2 = Premium, 3 = Recliner): ", &booking->categoryChoice)) {
            return 0;
        }

        if (booking->categoryChoice >= STANDARD_CATEGORY_CHOICE && booking->categoryChoice <= RECLINER_CATEGORY_CHOICE) {
            break;
        }

        printf("Error: Invalid category. Please choose 1, 2, or 3.\n");
    }

    return 1;
}

int selectTicketCategory(Booking *booking) {
    switch (booking->categoryChoice) {
        case STANDARD_CATEGORY_CHOICE:
            strcpy(booking->categoryName, "Standard");
            booking->ticketPrice = STANDARD_TICKET_PRICE;
            return 1;

        case PREMIUM_CATEGORY_CHOICE:
            strcpy(booking->categoryName, "Premium");
            booking->ticketPrice = PREMIUM_TICKET_PRICE;
            return 1;

        case RECLINER_CATEGORY_CHOICE:
            strcpy(booking->categoryName, "Recliner");
            booking->ticketPrice = RECLINER_TICKET_PRICE;
            return 1;

        default:
            return 0;
    }
}

void calculateBill(Booking *booking) {
    booking->subtotal = booking->ticketCount * booking->ticketPrice;

    if (booking->ticketCount >= DISCOUNT_ELIGIBILITY_COUNT) {
        booking->discountAmount = booking->subtotal * DISCOUNT_PERCENTAGE / PERCENTAGE_DIVISOR;
    } else {
        booking->discountAmount = 0.0;
    }

    booking->finalBill = booking->subtotal - booking->discountAmount;
}

void displayBookingDetails(const Booking *booking) {
    printf("\n----- Booking Details -----\n");
    printf("Customer: %s\n", booking->customerName);
    printf("Ticket Category: %s\n", booking->categoryName);
    printf("Tickets: %d\n", booking->ticketCount);
    printf("Subtotal: %.2f\n", booking->subtotal);
    printf("Discount: %.2f\n", booking->discountAmount);
    printf("Final Bill: %.2f\n", booking->finalBill);
}

int main(void) {
    Booking booking = {0};

    if (!readBookingDetails(&booking)) {
        return 1;
    }

    if (!selectTicketCategory(&booking)) {
        printf("Error: Invalid ticket category.\n");
        return 1;
    }

    calculateBill(&booking);
    displayBookingDetails(&booking);

    return 0;
}

/*
 1.readInteger() currently returns 1 only on successful input and keeps trying otherwise,
  can you explain the purpose of return value?

  Answer 1 :- 1 is used to represent success of reading means integer is accepted.
              and 0 is representing failure.

 2.why do you think BookingLimits needs to be an enum?
   Consider whether a different way of defining these constants would be more appropriate and why.

    Answer 2 :- We use enum when we encounter similar choices for ex:- movie ticket categories , yes we can also use #define
                but this #define is good when we have mixed choices or different constant values.

*/
