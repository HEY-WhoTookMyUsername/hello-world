#pragma once

namespace Yaml
{
	class YamlStream
	{
	public:
		static Ref<Yaml::Node> Read(class BinaryStream& stream);
		static bool Write(class BinaryStream& stream, Yaml::Node& rootNode);
	};
}