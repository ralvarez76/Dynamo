using System;
using NUnit.Framework;
using ProtoCore.DSASM.Mirror;
using ProtoTest.TD;
using ProtoTestFx.TD;
namespace ProtoTest.Assembler
{
    public class MicroFeatureTests
    {
        public ProtoCore.Core core;
        public TestFrameWork thisTest = new TestFrameWork();
        // private string AsmFilePath = @"..\..\..\Scripts\ASMScript\";
        [SetUp]
        public void Setup()
        {
            Console.WriteLine("Setup");
            core = new ProtoCore.Core(new ProtoCore.Options());
            core.Executives.Add(ProtoCore.Language.kAssociative, new ProtoAssociative.Executive(core));
            core.Executives.Add(ProtoCore.Language.kImperative, new ProtoImperative.Executive(core));
        }
        /*
[Test]
[Test]
[Test]
[Test]
[Test]
    }
}