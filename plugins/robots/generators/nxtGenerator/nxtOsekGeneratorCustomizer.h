#pragma once

#include <generatorBase/generatorCustomizer.h>
#include "nxtOsekGeneratorFactory.h"

namespace nxtOsek {

class NxtOsekGeneratorCustomizer : public generatorBase::GeneratorCustomizer
{
public:
	NxtOsekGeneratorCustomizer(qrRepo::RepoApi const &repo
			, qReal::ErrorReporterInterface &errorReporter);

	virtual generatorBase::GeneratorFactoryBase *factory();

private:
	NxtOsekGeneratorFactory mFactory;
};

}
