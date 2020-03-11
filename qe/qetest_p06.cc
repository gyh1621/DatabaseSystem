#include "qe_test_util.h"

RC privateTestCase_6() {
    RC rc = success;
    // Functions Tested
    // Left deep join using Index Nested Loop Join
    std::cerr << std::endl << "***** In QE Private Test Case 6 *****" << std::endl;

    // Prepare the iterator and condition
    auto *leftIn = new TableScan(rm, "largeleft2");
    auto *rightIn1 = new IndexScan(rm, "largeright2", "B");
    auto *rightIn2 = new IndexScan(rm, "left", "B");

    // Set up condition
    Condition filterCond;
    filterCond.lhsAttr = "largeleft2.B";
    filterCond.op = LE_OP;
    filterCond.bRhsIsAttr = false;
    Value value1{};
    value1.type = TypeInt;
    value1.data = malloc(bufSize);
    *(int *) value1.data = 109;
    filterCond.rhsValue = value1;

    // Create Filter
    auto *filter = new Filter(leftIn, filterCond);

    Condition joinCond1;
    joinCond1.lhsAttr = "largeleft2.B";
    joinCond1.op = EQ_OP;
    joinCond1.bRhsIsAttr = true;
    joinCond1.rhsAttr = "largeright2.B";

    // Create child NLJoin
    auto *childINLJoin = new INLJoin(filter, rightIn1, joinCond1);

    Condition joinCond2;
    joinCond2.lhsAttr = "largeleft2.B";
    joinCond2.op = EQ_OP;
    joinCond2.bRhsIsAttr = true;
    joinCond2.rhsAttr = "left.B";

    // Create top NLJoin
    auto *inlJoin = new INLJoin(childINLJoin, rightIn2, joinCond2);

    int expectedResultCnt = 90; //20~109 --> largeleft2.B: [10,109], largeright2.B: [20,50019], left.B: [10, 109]
    int actualResultCnt = 0;
    int valueB = 0;

    // Go over the data through iterator
    void *data = malloc(bufSize);
    while (inlJoin->getNextTuple(data) != QE_EOF) {
        if (actualResultCnt % 10 == 0) {

            int offset = 2; //including nulls-indicator

            // Print largeLeft.A
            std::cerr << "largeleft2.A " << *(int *) ((char *) data + offset);
            offset += sizeof(int);

            // Print largeLeft.B
            std::cerr << " B " << *(int *) ((char *) data + offset);
            offset += sizeof(int);

            // Print largeLeft.C
            std::cerr << " C " << *(float *) ((char *) data + offset);
            offset += sizeof(float);

            // Print largeLeft.B
            valueB = *(int *) ((char *) data + offset);
            std::cerr << " largeright2.B " << valueB;
            offset += sizeof(int);

            if (valueB < 20 || valueB > 109) {
                std::cerr << std::endl << "***** [FAIL] Incorrect value: " << valueB << " returned. *****" << std::endl;
                rc = fail;
                break;
            }

            // Print largeLeft.C
            std::cerr << " C " << *(float *) ((char *) data + offset);
            offset += sizeof(float);

            // Print largeLeft.D
            std::cerr << " D " << *(int *) ((char *) data + offset);
            offset += sizeof(int);

            // Print left.A
            std::cerr << " left.A " << *(int *) ((char *) data + offset);
            offset += sizeof(int);

            // Print largeLeft.B
            std::cerr << " B " << *(int *) ((char *) data + offset);
            offset += sizeof(int);

            // Print largeLeft.C
            std::cerr << " C " << *(float *) ((char *) data + offset) << std::endl;
        }

        memset(data, 0, bufSize);
        ++actualResultCnt;
    }

    if (expectedResultCnt != actualResultCnt) {
        std::cerr << " ***** Expected Result Count: " << expectedResultCnt << std::endl;
        std::cerr << " ***** [FAIL] The number of result: " << actualResultCnt << " is not correct. ***** "
                  << std::endl;
        rc = fail;
    }

    delete inlJoin;
    delete childINLJoin;
    delete leftIn;
    delete rightIn1;
    delete rightIn2;
    delete filter;
    free(value1.data);
    free(data);
    return rc;
}

int main() {

    if (privateTestCase_6() != success) {
        std::cerr << "***** [FAIL] QE Private Test Case 6 failed. *****" << std::endl;
        return fail;
    } else {
        std::cerr << "***** QE Private Test Case 6 finished. The result will be examined. *****" << std::endl;
        return success;
    }
}
