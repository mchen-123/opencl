typedef struct iDFDevice *DFDevice;
typedef struct iDFcontext *DFContext;
typedef struct iDFstream *DFStream;
typedef struct iDFdeviceptr *DFDeviceptr;
typedef struct iDFModule *DFModule;
typedef struct iDFfunction *DFFunction;

typedef enum {
  DF_DEV_ATTR_MEM_INFO,                  ///< Device memory info, see also dfDeviceTotalMem()
  DF_DEV_ATTR_DIE_MEM_INFO,              ///< Device die memory info, see also dfDeviceTotalMemEx()
  DF_DEV_ATTR_GRID_INFO,                 ///< Device grid info, see also dfDeviceGetDieGrid()
  DF_DEV_ATTR_CLOCK_RATE,                ///< Device clock rate
  DF_DEV_ATTR_COMPUTE_CAPABILITY_MAJOR,  ///< Device major compute capability version number
  DF_DEV_ATTR_COMPUTE_CAPABILITY_MINOR,  ///< Device minor compute capability version number
  DF_DEV_ATTR_CAN_USE_DEV_PTR_IN_HOST,   ///< Host can use device pointer to access dev
                                         ///< memory
  DF_DEV_ATTR_PE_COUNT_IN_ONE_DIE,       ///< PE count in one die
  DF_DEV_ATTR_RV_COUNT_IN_ONE_PE,        ///< RV count in one PE
} DFDeviceAttribute;

/**
 * @struct DFDieCoord
 * @brief PE die coordinate
 */
typedef struct {
  int x;  ///< PE die coordinate x
  int y;  ///< PE die coordinate y
} DFDieCoord;

/**
 * @struct DFDieGrid
 * @brief PE die grid
 */
typedef struct {
  DFDieCoord upperLeft;    ///< Upper left corner of the die grid
  DFDieCoord bottomRight;  ///< Bottom right corner of the die grid
} DFDieGrid;

/**
 * @struct DFDieCoords
 * @brief PE die array
 */
typedef struct {
  int count;         ///< Number of die coordinates in the array
  DFDieCoord *dies;  ///< Array of die coordinates
} DFDieCoords;

/**
 * @enum DFDieConfigType
 * @brief Type of die configuration
 */
typedef enum {
  DF_DIE_CONFIG_TYPE_GRID,    ///< PE die grid configuration
  DF_DIE_CONFIG_TYPE_COORDS,  ///< PE die array configuration using coordinates, not supported yet
} DFDieConfigType;

typedef struct {
  DFDieConfigType type;  ///< Type of die configuration
  union {
    DFDieGrid grid;      ///< PE die grid configuration
    DFDieCoords coords;  ///< PE die array configuration using coordinates
  };
} DFDieConfig;

/**
 * @enum DFMemcpyFlag
 * @brief Memory copy flag
 */
typedef enum {
  DF_MEMCOPY_SPLIT,     ///< Split source data to specified PEs
  DF_MEMCOPY_BROADCAST  ///< Broadcast source data to specified PEs
} DFMemcpyFlag;