set(MODELS_SRC
    # common sources
    ${THERMCORE_SOURCE_DIR}/common/atherm_common.cpp
    # gas_parameters sources
    ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description.cpp
    ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description_dynamic.cpp
    ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description_static.cpp
    ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_ng_gost30319.cpp
    ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_ng_gost56851.cpp
    ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_ng_gost_defines.cpp
    ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description_mix.cpp
    # phase_diagram sources
    ${THERMCORE_SOURCE_DIR}/phase_diagram/phase_diagram.cpp
    ${THERMCORE_SOURCE_DIR}/phase_diagram/phase_diagram_models.cpp
    # subroutins sources
    ${THERMCORE_SOURCE_DIR}/subroutins/file_structs.cpp
    # service sources
    ${THERMCORE_SOURCE_DIR}/service/calculation_info.cpp
    ${THERMCORE_SOURCE_DIR}/service/calculation_setup.cpp
    ${THERMCORE_SOURCE_DIR}/service/program_state.cpp
    # models sources
    ${THERMCORE_SOURCE_DIR}/models/model_general.cpp
    ${THERMCORE_SOURCE_DIR}/models/model_ideal_gas.cpp
    ${THERMCORE_SOURCE_DIR}/models/model_ng_gost.cpp
    ${THERMCORE_SOURCE_DIR}/models/model_peng_robinson.cpp
    ${THERMCORE_SOURCE_DIR}/models/model_redlich_kwong.cpp
    ${THERMCORE_SOURCE_DIR}/models/model_redlich_kwong_soave.cpp
    ${THERMCORE_SOURCE_DIR}/models/models_configurations.cpp
    ${THERMCORE_SOURCE_DIR}/models/models_creator.cpp
    # database sources
    ${THERMDB_SOURCE_DIR}/atherm_db_tables.cpp)
