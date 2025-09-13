#include <stdinclude.hpp>

void* UnitIdol::field_UnitIdol_charaId = nullptr;
void* UnitIdol::klass_UnitIdol = nullptr;
void* UnitIdol::field_UnitIdol_clothId = nullptr;
void* UnitIdol::field_UnitIdol_hairId = nullptr;
void* UnitIdol::field_UnitIdol_accessoryIds = nullptr;

void UnitIdol::ReadFrom(void* managed) {
	if (AccessoryIds != nullptr) {
		delete[] AccessoryIds;
	}
	InitUnitIdol(managed);
	void* accessoryIds;
	il2cpp_field_get_value(managed, field_UnitIdol_charaId, &CharaId);
	il2cpp_field_get_value(managed, field_UnitIdol_clothId, &ClothId);
	il2cpp_field_get_value(managed, field_UnitIdol_hairId, &HairId);
	il2cpp_field_get_value(managed, field_UnitIdol_accessoryIds, &accessoryIds);
	AccessoryIdsLength = il2cpp_array_length(accessoryIds);
	AccessoryIds = new int[AccessoryIdsLength];
	for (int i = 0; i < AccessoryIdsLength; ++i) {
		auto item = il2cpp_symbols::array_get_value(accessoryIds, i);
		int32_t* rawPtr = static_cast<int32_t*>(il2cpp_object_unbox(item));
		int32_t value = *rawPtr;
		AccessoryIds[i] = value;
	}
}

void UnitIdol::ApplyTo(void* managed) {
	InitUnitIdol(managed);
	il2cpp_field_set_value(managed, field_UnitIdol_charaId, &CharaId);
	il2cpp_field_set_value(managed, field_UnitIdol_clothId, &ClothId);
	il2cpp_field_set_value(managed, field_UnitIdol_hairId, &HairId);
	static auto klass_System_Int32 = il2cpp_class_from_name(il2cpp_get_corlib(), "System", "Int32");
	auto accessoryIds = il2cpp_array_new(klass_System_Int32, AccessoryIdsLength);
	auto length = il2cpp_array_length(accessoryIds);
	for (int i = 0; i < AccessoryIdsLength; ++i) {
		auto boxed = il2cpp_value_box(klass_System_Int32, &AccessoryIds[i]);
		il2cpp_symbols::array_set_value(accessoryIds, boxed, i);
	}
	il2cpp_field_set_value_object(managed, field_UnitIdol_accessoryIds, accessoryIds);
}

void UnitIdol::Print(std::ostream& os) const {
	os << "{ CharaId = " << CharaId
		<< ", ClothId = " << ClothId
		<< ", HairId = " << HairId
		<< ", AccessoryIds = [";
	bool first = true;
	if (AccessoryIds != nullptr) {
		for (int i = 0; i < AccessoryIdsLength; ++i) {
			if (!first)
				os << ", ";
			os << AccessoryIds[i];
			first = false;
		}
	}
	os << "] }" << std::endl;
}

std::string UnitIdol::ToString() const {
	std::ostringstream oss;
	Print(oss);
	return oss.str();
}


bool tools::output_networking_calls = false;
