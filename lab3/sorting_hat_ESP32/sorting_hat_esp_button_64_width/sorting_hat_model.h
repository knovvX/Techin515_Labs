#pragma once
#include <cstdarg>
namespace Eloquent {
    namespace ML {
        namespace Port {
            class DecisionTree {
                public:
                    /**
                    * Predict class for features vector
                    */
                    int predict(float *x) {
                        if (x[1] <= 1.5) {
                            if (x[9] <= 0.5) {
                                return 0;
                            }

                            else {
                                if (x[0] <= 1.5) {
                                    return 1;
                                }

                                else {
                                    return 0;
                                }
                            }
                        }

                        else {
                            if (x[1] <= 2.5) {
                                return 2;
                            }

                            else {
                                if (x[4] <= 0.5) {
                                    return 2;
                                }

                                else {
                                    return 3;
                                }
                            }
                        }
                    }

                protected:
                };
            }
        }
    }