//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//

#pragma once

#include "ModelH3D.h"

class AssimpModel : public ModelH3D
{
public:

	enum
	{
		format_none = 0,
		format_h3d, // native format
        format_mini,

		formats,
	};
	static const char *s_FormatString[];
	static int FormatFromFilename(const std::string& filename);

	bool Load(const std::string& filename);
	bool Save(const std::string& filename) const;

    void PrintModelStats();

private:

	bool LoadAssimp(const std::string& filename);

	void Optimize();
	void OptimizeRemoveDuplicateVertices(bool depth);
	void OptimizePostTransform(bool depth);
	void OptimizePreTransform(bool depth);
};

