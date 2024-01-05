//
//  DuckDB
//  https://github.com/duckdb/duckdb-swift
//
//  Copyright © 2018-2024 Stichting DuckDB Foundation
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to
//  deal in the Software without restriction, including without limitation the
//  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
//  sell copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
//  IN THE SOFTWARE.

public struct CodingUserInfoKeys {
  /// This key is set on the `userInfo` dictionary of the `Decoder` that is
  /// used when transforming data into a `Decodable`. The value is the ``LogicalType``
  /// of the element being decoded. This can be used to implement dynamic decoding
  /// behavior based on the underlying database type.
  ///
  /// For example:
  /// ```swift
  ///  struct DynamicDecodable: Decodable {
  ///   init(from decoder: Decoder) throws {
  ///      guard let logicalType = decoder.userInfo[CodingUserInfoKeys.logicalType] as? LogicalType else {
  ///       throw Error.expectedLogicalType
  ///      }
  ///      switch logicalType.dataType {
  ///        case .list:
  ///          let unkeyedContainer = try decoder.unkeyedContainer()
  ///          ...
  ///        case .map, .struct:
  ///          let keyedContainer = try decoder.container(keyedBy: AnyCodingKey.self)
  ///          ...
  ///      }
  ///    }
  ///  }
  ///
  ///  let column = result[0].cast(to: DynamicDecodable.self)
  ///  ```
  public static let logicalTypeCodingUserInfoKey = CodingUserInfoKey(rawValue: "logicalType")!
}
