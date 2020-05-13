#pragma once
#include <assert.h>

#define D3DSAFERELEASE(ptr) if((ptr)) (ptr)->Release()


// Assertions
#define ASSERT_HRESULT_SUCCESS(result) assert(!FAILED((result)))